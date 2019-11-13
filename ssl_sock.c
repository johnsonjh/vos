/*
 Based on example from
     https://github.com/benvanik/openssl/blob/master/openssl/demos/state_machine/state_machine.c

*/

#include "vos.h"

#define die_unless(x) assert(x)

#define fprintf null

void inline null(FILE *f,...) {}


void SSLStateMachine_print_error(SSLStateMachine *pMachine,const char *szErr)
    {
    unsigned long l;

    fprintf(stderr,"%s\n",szErr);
    while((l=ERR_get_error()))
	{
	char buf[1024];

	ERR_error_string_n(l,buf,sizeof buf);
	fprintf(stderr,"Error %lx: %s\n",l,buf);
	}
}

void  SSLStateMachine_done(SSLStateMachine *pMachine) {
    if (pMachine->pSSL) SSL_free(pMachine->pSSL);
    if (pMachine->pCtx) SSL_CTX_free(pMachine->pCtx);
    free(pMachine);
}

SSLStateMachine *SSLStateMachine_new(const char *szCertificateFile,
				     const char *szKeyFile,int isServer)
{
    SSLStateMachine *pMachine=malloc(sizeof *pMachine);
    int n;
    memset(pMachine,0,sizeof(*pMachine));
//printf("m=%p\n",pMachine);
// SSL_library_init();
 //OPENSSL_add_all_algorithms_noconf();
    if (!pMachine) { SSLStateMachine_done(pMachine); return 0;}
    //die_unless(pMachine);

    //pMachine->pCtx=SSL_CTX_new(SSLv23_server_method());
//printf("2\n");
     if ( isServer )
        pMachine->pCtx=SSL_CTX_new(SSLv23_server_method());
    else
        pMachine->pCtx=SSL_CTX_new(SSLv23_client_method());
//printf("m=%d\n",pMachine->pCtx);
    if (!pMachine->pCtx) { SSLStateMachine_done(pMachine); return 0;}
    //die_unless(pMachine->pCtx);
    if (szCertificateFile) {
    n=SSL_CTX_use_certificate_file(pMachine->pCtx,szCertificateFile,
				   SSL_FILETYPE_PEM);

    if (n<=0) { SSLStateMachine_done(pMachine); return 0;}
    }
//printf("3\n");

    if (szKeyFile) {
    n=SSL_CTX_use_PrivateKey_file(pMachine->pCtx,szKeyFile,SSL_FILETYPE_PEM);
    if (n<=0) { SSLStateMachine_done(pMachine); return 0;}
    }

    pMachine->pSSL=SSL_new(pMachine->pCtx);
//printf("4\n");
    //die_unless(pMachine->pSSL);
    if (!pMachine->pSSL) { SSLStateMachine_done(pMachine); return 0;}
//printf("5\n");

    pMachine->pbioRead=BIO_new(BIO_s_mem());

    pMachine->pbioWrite=BIO_new(BIO_s_mem());
//printf("6\n");

    SSL_set_bio(pMachine->pSSL,pMachine->pbioRead,pMachine->pbioWrite);

    //SSL_set_accept_state(pMachine->pSSL);
//printf("7\n");
    if ( isServer )
        SSL_set_accept_state(pMachine->pSSL);
    else {
        SSL_set_connect_state(pMachine->pSSL);
        SSL_do_handshake(pMachine->pSSL);
        }
    return pMachine;
}

int ssl_init_finished(SSLStateMachine *pMachine) { return SSL_is_init_finished(pMachine->pSSL); }

void SSLStateMachine_read_inject(SSLStateMachine *pMachine,
				 const unsigned char *aucBuf,int nBuf)
    {
    int n=BIO_write(pMachine->pbioRead,aucBuf,nBuf);
    /* If it turns out this assert fails, then buffer the data here
     * and just feed it in in churn instead. Seems to me that it
     * should be guaranteed to succeed, though.
     */
    assert(n == nBuf);
    fprintf(stderr,"%d bytes of encrypted data fed to state machine\n",n);
    }

int SSLStateMachine_read_extract(SSLStateMachine *pMachine,
				 unsigned char *aucBuf,int nBuf)
    {
    int n=0;

    if(!SSL_is_init_finished(pMachine->pSSL))
	{
	fprintf(stderr,"Doing SSL_accept\n");
	if (pMachine->isServer) n=SSL_accept(pMachine->pSSL);
	      else {
           // SSL_do_handshake(pMachine->pSSL);
	       n=SSL_connect(pMachine->pSSL); //SSL_do_handshake(pMachine->pSSL);
	       }
	if(n == 0)
	    fprintf(stderr,"SSL_accept returned zero\n");
	if(n < 0)
	    {
	    int err;

	    if((err=SSL_get_error(pMachine->pSSL,n)) == SSL_ERROR_WANT_READ)
		{
		fprintf(stderr,"SSL_accept wants more data\n");
		return 0;
		}

	     SSLStateMachine_print_error(pMachine,"SSL_accept error");
	     exit(7);
	    }
	return 0;
	}

    n=SSL_read(pMachine->pSSL,aucBuf,nBuf);
    if(n < 0)
	{
	int err=SSL_get_error(pMachine->pSSL,n);

	if(err == SSL_ERROR_WANT_READ)
	    {
	    fprintf(stderr,"SSL_read wants more data\n");
	    return 0;
	    }

	SSLStateMachine_print_error(pMachine,"SSL_read error");
	exit(8); // ZUZU!
	}

    fprintf(stderr,"%d bytes of decrypted data read from state machine\n",n);
    //hex_dump("decrypted_read",aucBuf,n);
    return n;
    }

int SSLStateMachine_write_can_extract(SSLStateMachine *pMachine)
    {
    int n=BIO_pending(pMachine->pbioWrite);
    if(n)
	fprintf(stderr,"There is encrypted data available to write\n");
    else
	  fprintf(stderr,"There is no encrypted data available to write\n");

    return n;
    }

int SSLStateMachine_write_extract(SSLStateMachine *pMachine,
				  unsigned char *aucBuf,int nBuf)
    {
    int n;

    n=BIO_read(pMachine->pbioWrite,aucBuf,nBuf);
    fprintf(stderr,"%d bytes of encrypted data read from state machine on write_extract\n",n);
    return n;
    }

void SSLStateMachine_write_inject(SSLStateMachine *pMachine,
				  const unsigned char *aucBuf,int nBuf)
    {
    int n=SSL_write(pMachine->pSSL,aucBuf,nBuf);
    /* If it turns out this assert fails, then buffer the data here
     * and just feed it in in churn instead. Seems to me that it
     * should be guaranteed to succeed, though.
     */
    //    fprintf(stderr,"%d bytes of unencrypted data fed to state machine\n",n);
    assert(n == nBuf);
    fprintf(stderr,"%d bytes of unencrypted data fed to state machine\n",n);
}


void ssl_sock_init() {
SSL_library_init();
    OpenSSL_add_ssl_algorithms();
    SSL_load_error_strings();
ERR_load_crypto_strings();
}
