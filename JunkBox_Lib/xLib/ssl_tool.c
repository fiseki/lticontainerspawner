
/**
@brief   SSL暗号用ライブラリ
@file    ssl_tool.c
@author  Fumi.Iseki (C)

@par ライブラリ
--L/usr/local/ssl/lib -lssl -lcrypto 
--L/usr/lib64 -lssl -lcrypto -lcrypt

@par 転送データ（テキスト）形式
@code
通常転送データ：            [データ][\r][\n]
通常転送データ（複数行）：  [データ][\r][\n][データ][\r][\n]...[\r][\n][END][\r][\n]
暗号化時の転送データ：      [暗号化データ][\r][\n] （一行）
                                   |→復号→ [データ][\r][\n]....[\r][\n][END][\r][\n]
注）データを転送するとき，\r\n が生のデータの最後につくのか，暗号化されたデータの最後につくのか，よく注意しないといけない．
@endcode

@attention
このプログラムは openSSL を使用しています．@n
This product includes software developed by the OpenSSL Project
for use in the OpenSSL Toolkit. (http://www.openssl.org/)
*/

#include "ssl_tool.h"


#ifdef ENABLE_SSL


//Buffer* Base64_DHspki  = NULL;
//Buffer* Base64_RSAspki = NULL;

//Buffer* CRYPT_SharedKey        = NULL;
//const   EVP_CIPHER* CRYPT_Type = NULL;
//DH*     DHkey                  = NULL;

//int     KEYEX_Algorism = 0;
//int     CRYPT_Algorism = 0;


/*
typedef struct dh_st
{
    BIGNUM* p;
    BIGNUM* g;
    BIGNUM* pub_key;
    BIGNUM* priv_key; 
} DH;
 */


/*
###############################################################################################
for OpenSSL3
see https://stackoverflow.com/questions/71551116/openssl-3-diffie-hellman-key-exchange-c
see https://github.com/eugen15/diffie-hellman-cpp
###############################################################################################
*/


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//

/**
int  gen_CRYPT_SharedKey(int keyex, Buffer spki, Buffer* shdkey, void* ptr)

鍵交換アルゴリズム keyex 下で，共有鍵を計算する．

自分のSPKIと秘密鍵は計算済みでなければならない．

@b keyex @n
@b SSL_SH: Diffie-Hellman の場合は，DHの秘密鍵が，DH_generate_key()（または gen_DHspki(), gnen_DHspki_fs(), 
get_DHspki_ff()でも良い）により計算（またはロード）され，void* ptr に格納されていなければならない．@n
ptr = (void*)dhkey @n
@b RSA: RSAの場合は............@n

@param  keyex   鍵交換アルゴリズム．現在サポートしているのは SSL_DH のみ．
@param  spki    相手の SPKI 
@param  shdkey  生成された共有鍵
@param  ptr     付加情報（SSL_DHの場合は DH* ptr）

@retval TRUE   成功
@retval FALSE  失敗

@see DH_generate_key()
*/
int  gen_CRYPT_SharedKey(int keyex, Buffer spki, Buffer* shdkey, void* ptr)
{
    if (spki.buf==NULL || shdkey==NULL)  return FALSE;

    if (keyex==SSL_DH)  {
        DH* dhkey = (DH*)ptr;
        *shdkey = get_DHsharedkey(spki, dhkey);     // 共有鍵を得る
    }
    else if (keyex==SSL_RSA) {                      // 未実装
        *shdkey = dup_Buffer(spki); 
    }
    else return FALSE;

    return TRUE;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////
//

int   udp_send_crypt_Buffer_sockaddr_in(int sock, Buffer* data,  struct sockaddr_in* sv, Buffer* key, EVP_CIPHER* cipher)
{
    int  cc = 0;
    Buffer buf;

    if (data==NULL) return JBXL_ARGS_ERROR;

    if (key!=NULL && cipher!=NULL) {
        buf = encode_EVPAPI_Buffer(*data, *key, cipher);
        cc = udp_send_Buffer_sockaddr_in(sock, &buf, sv);
        free_Buffer(&buf);
    }
    else {
        cc = udp_send_Buffer_sockaddr_in(sock, data, sv);
    }

    return cc;
}



int  udp_recv_crypt_Buffer_sockaddr_in(int sock, Buffer* data, struct sockaddr_in* sv, Buffer* key, EVP_CIPHER* cipher)
{  
    Buffer  buf;

    buf = make_Buffer(RECVBUFSZ);
    int cc = udp_recv_Buffer_sockaddr_in(sock, &buf, sv);
    if (cc<=0) {
        free_Buffer(&buf);
        return cc;
    }
   
    if (key!=NULL && cipher!=NULL) {
        Buffer dec = decode_EVPAPI_Buffer(buf, *key, cipher);
        copy_Buffer(&dec, data);
        free_Buffer(&dec);
    }
    else {
        copy_Buffer(&buf, data);
    }

    cc = data->vldsz;
    free_Buffer(&buf);
    return cc;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////
//

int   udp_send_crypt_Buffer(int sock, Buffer* data,  struct addrinfo* sv, Buffer* key, EVP_CIPHER* cipher)
{
    int  cc = 0;
    Buffer buf;

    if (data==NULL) return JBXL_ARGS_ERROR;

    if (key!=NULL && cipher!=NULL) {
        buf = encode_EVPAPI_Buffer(*data, *key, cipher);
        cc = udp_send_Buffer(sock, &buf, sv);
        free_Buffer(&buf);
    }
    else {
        cc = udp_send_Buffer(sock, data, sv);
    }

    return cc;
}



int  udp_recv_crypt_Buffer(int sock, Buffer* data, struct addrinfo* sv, Buffer* key, EVP_CIPHER* cipher)
{  
    Buffer  buf;

    buf = make_Buffer(RECVBUFSZ);
    int cc = udp_recv_Buffer(sock, &buf, sv);
    if (cc<=0) {
        free_Buffer(&buf);
        return cc;
    }
   
    if (key!=NULL && cipher!=NULL) {
        Buffer dec = decode_EVPAPI_Buffer(buf, *key, cipher);
        copy_Buffer(&dec, data);
        free_Buffer(&dec);
    }
    else {
        copy_Buffer(&buf, data);
    }

    cc = data->vldsz;
    free_Buffer(&buf);
    return cc;
}



int  tcp_send_crypt_Buffer(int sock, Buffer* data, Buffer* key, EVP_CIPHER* cipher)
{
    int  cc = 0;
    Buffer buf;

    if (data==NULL) return JBXL_ARGS_ERROR;

    if (key!=NULL && cipher!=NULL) {
        buf = encode_EVPAPI_Buffer(*data, *key, cipher);
        cc = tcp_send_Buffer(sock, &buf);
        free_Buffer(&buf);
    }
    else {
        cc = tcp_send_Buffer(sock, data);
    }

    return cc;
}



int  tcp_recv_crypt_Buffer(int sock, Buffer* data, Buffer* key, EVP_CIPHER* cipher)
{  
    Buffer  buf;

    buf = make_Buffer(RECVBUFSZ);
    int cc = tcp_recv_Buffer(sock, &buf);
    if (cc<=0) {
        free_Buffer(&buf);
        return cc;
    }
   
    if (key!=NULL && cipher!=NULL) {
        Buffer dec = decode_EVPAPI_Buffer(buf, *key, cipher);
        copy_Buffer(&dec, data);
        free_Buffer(&dec);
    }
    else {
        copy_Buffer(&buf, data);
    }

    cc = data->vldsz;
    free_Buffer(&buf);
    return cc;
}



Buffer  get_plain_Buffer(Buffer data, Buffer* key, EVP_CIPHER* cipher)
{
    Buffer buf = init_Buffer();
    if (key==NULL || cipher==NULL) return buf;

    buf = decode_EVPAPI_Buffer(data, *key, cipher);
    return buf;
}



Buffer  get_crypt_Buffer(Buffer data, Buffer* key, EVP_CIPHER* cipher)
{
    Buffer buf = init_Buffer();
    if (key==NULL || cipher==NULL) return buf;

    buf = encode_EVPAPI_Buffer(data, *key, cipher);
    return buf;
}



/**
int  tcp_send_crypt_mesg(int sock, char* mesg, Buffer* key, EVP_CIPHER* cipher)

暗号化対応の転送関数．

暗号化データの最後には必ず @\r@\n がつく．生データの最後の @\r@\n はユーザの責任で付加する（通常はつける）．
CRYPT_Algorism が 0 でない場合には暗号化(+@\r@\n)して送られる．

tcp_send_crypt_mesgln(), tcp_send_crypt_sBufferln() は最後の改行コードの
取り扱いが混乱するので定義しない方が良い（と思う）．定義する場合は tcp_send_mesgln(),
tcp_send_sBufferln()とは使い方が異なることになるので注意が必要．

@param  sock    ソケット
@param  mesg    転送するデータ．
@param  key     暗号鍵へのポインタ．
@param  cipher  暗号方式

@retval 0以上   転送データ数
@retval 0未満   通信エラー

@code
tcp_send_crypt_mesg(sock, "OK\r\n");
@endcode
*/
int  tcp_send_crypt_mesg(int sock, char* mesg, Buffer* key, EVP_CIPHER* cipher)
{
    int  cc = 0;
    Buffer buf, enc;

    if (mesg==NULL) return JBXL_ARGS_ERROR;
    //DEBUG_MODE PRINT_ESC(" SEND -> [%s]\n", mesg);

    if (key!=NULL && cipher!=NULL) {
        buf = make_Buffer_bystr(mesg);
        enc = encode_EVPAPI_Buffer(buf, *key, cipher);
        free_Buffer(&buf);
        if (enc.vldsz>0) {
            buf = encode_base64_Buffer(enc);
            free_Buffer(&enc);

            //DEBUG_MODE PRINT_ESC("CSEND -> [%s]\n", (char*)buf.buf);
            cc = tcp_send_sBufferln(sock, &buf);
            free_Buffer(&buf);
        }
    }
    else {
        cc = tcp_send_mesg(sock, mesg);
    }

    return cc;
}



/**
int  tcp_send_crypt_sBuffer(int sock, Buffer* mesg, Buffer* key, EVP_CIPHER* cipher)

暗号化対応の転送関数．

暗号化データの最後には必ず @\r@\n がつく．生データの最後の @\r@\n はユーザの責任で付加する（通常はつける）．
CRYPT_Algorism が 0 でない場合には暗号化(+@\r@\n)して送られる．

tcp_send_crypt_mesgln(), tcp_send_crypt_sBufferln() は最後の改行コードの
取り扱いが混乱するので定義しない方が良い（と思う）．定義する場合は tcp_send_mesgln(),
tcp_send_sBufferln()とは使い方が異なることになるので注意が必要．

@param  sock    ソケット
@param  mesg    転送するデータ．
@param  key     暗号鍵へのポインタ．
@param  cipher  暗号方式

@retval 0以上   転送データ数
@retval 0未満   通信エラー

@code
tcp_send_crypt_mesg(sock, "OK\r\n");
@endcode
*/
int  tcp_send_crypt_sBuffer(int sock, Buffer* mesg, Buffer* key, EVP_CIPHER* cipher)
{
    int  cc = 0;
    Buffer buf, enc;

    if (mesg==NULL) return JBXL_ARGS_ERROR;
    //DEBUG_MODE PRINT_ESC(" SEND -> [%s]\n", (char*)mesg->buf);

    if (key!=NULL && cipher!=NULL) {
        enc = encode_EVPAPI_Buffer(*mesg, *key, cipher);
        if (enc.vldsz>0) {
            buf = encode_base64_Buffer(enc);
            free_Buffer(&enc);

            //DEBUG_MODE PRINT_ESC("CSEND -> [%s]\n", (char*)buf.buf);
            cc = tcp_send_sBufferln(sock, &buf);
            free_Buffer(&buf);
        }
    }
    else {
        cc = tcp_send_sBuffer(sock, mesg);
    }

    return cc;
}



/**
Buffer  get_plain_message(char* mesg, Buffer* key, EVP_CIPHER* cipher)

復号化関数．復号の前に Base64のコードを行なう．

@b CRYPT_Algorism が 0 の場合は変換を行なわない． 

@param  mesg    変換するメッセージ
@param  key     暗号鍵へのポインタ．
@param  cipher  暗号方式

@return 変換されたメッセージ
*/
Buffer  get_plain_message(char* mesg, Buffer* key, EVP_CIPHER* cipher)
{
    Buffer  buf, dec;

    buf = make_Buffer_bystr(mesg);
    //DEBUG_MODE PRINT_ESC(" MESG -> [%s]\n", mesg);
    if (key==NULL || cipher==NULL) return buf;

    dec = decode_base64_Buffer(buf);
    if (dec.vldsz>0) {
        free_Buffer(&buf);
        buf = decode_EVPAPI_Buffer(dec, *key, cipher);
        free_Buffer(&dec);
        //DEBUG_MODE PRINT_ESC("DMESG -> [%s]\n", (char*)buf.buf);
    }
    return buf;
}



/**
Buffer  get_plain_sBuffer(Buffer mesg, Buffer* key, EVP_CIPHER* cipher)

復号化関数．復号の前に Base64のコードを行なう．

@b CRYPT_Algorism が 0 の場合は変換を行なわない． 

@param  mesg    変換するメッセージ
@param  key     暗号鍵へのポインタ．
@param  cipher  暗号方式

@return 変換されたメッセージ
*/
Buffer  get_plain_sBuffer(Buffer mesg, Buffer* key, EVP_CIPHER* cipher)
{
    Buffer buf, dec;

    buf = dup_Buffer(mesg);
    //DEBUG_MODE PRINT_ESC(" MESG -> [%s]\n", (char*)mesg.buf);
    if (key==NULL || cipher==NULL) return buf;

    dec = decode_base64_Buffer(buf);
    if (dec.vldsz>0) {
        free_Buffer(&buf);
        buf = decode_EVPAPI_Buffer(dec, *key, cipher);
        free_Buffer(&dec);
        //DEBUG_MODE PRINT_ESC("DMESG -> [%s]\n", (char*)buf.buf);
    }
    return buf;
}



/**
Buffer  get_crypt_message(char* mesg, Buffer* key, EVP_CIPHER* cipher)

暗号化関数．暗号化に加えて Base64符号化も行なう．

@b CRYPT_Algorism が 0 の場合は変換を行なわない． 

@param  mesg    変換するメッセージ
@param  key     暗号鍵へのポインタ．
@param  cipher  暗号方式

@return 変換されたメッセージ
*/
Buffer  get_crypt_message(char* mesg, Buffer* key, EVP_CIPHER* cipher)
{
    Buffer  buf, enc;

    buf = make_Buffer_bystr(mesg);
    if (key==NULL || cipher==NULL) return buf;

    enc = encode_EVPAPI_Buffer(buf, *key, cipher);
    if (enc.vldsz>0) {
        free_Buffer(&buf);
        buf = encode_base64_Buffer(enc);
        free_Buffer(&enc);
    }

    return buf;
}



/**
Buffer  get_crypt_sBuffer(Buffer mesg, Buffer* key, EVP_CIPHER* cipher)

復号化関数．暗号化に加えて Base64符号化も行なう．

@b CRYPT_Algorism が 0 の場合は変換を行なわない． 

@param  mesg    変換するメッセージ
@param  key     暗号鍵へのポインタ．
@param  cipher  暗号方式

@return 変換されたメッセージ
*/
Buffer  get_crypt_sBuffer(Buffer mesg, Buffer* key, EVP_CIPHER* cipher)
{
    Buffer  buf, enc;

    buf = dup_Buffer(mesg);
    if (key==NULL || cipher==NULL) return buf;

    enc = encode_EVPAPI_Buffer(buf, *key, cipher);
    if (enc.vldsz>0) {
        free_Buffer(&buf);
        buf = encode_base64_Buffer(enc);
        free_Buffer(&enc);
    }

    return buf;
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Client's Side Check 
//

/**
int  check_server_spki(char* filename, Buffer ipaddr, Buffer spki)

サーバから得た SPKIを，以前保存しておいた SPKIと比較して，サーバの正当性を検査する．

ファイル中にSPKIが無い場合は，新規のサーバとして，IPアドレスとSPKIを保存する．
クライアント側でサーバの認識に使用する．

@param  filename  鍵が保存されているファイル
@param  ipaddr    サーバのIPアドレス（文字列型）例：192.168.1.1
@param  spki      サーバのSPKI(DER)．

@retval TRUE      サーバのSPKIは以前保存したSPKIに一致した．または新規のサーバ．
@retval FALSE     サーバのSPKIは以前保存したSPKIに一致しない．
 */
int  check_server_spki(Buffer ipaddr, Buffer spki, char* filename)
{
    Buffer  buf;
    FILE*    fp;

    buf = init_Buffer();

    fp = fopen(filename, "rb");
    if (fp!=NULL) {
        buf = read_spki_with_ipaddr(ipaddr, fp);
        fclose(fp);
        if (buf.buf!=NULL) {
            if (bincmp_Buffer(buf, spki)) {
                free_Buffer(&buf);
                return FALSE;
            }
        }
    }

    if (buf.buf==NULL) {
        fp = file_chmod_open(filename, (char*)"a", S_IRUSR | S_IWUSR);
        save_spki_with_ipaddr(ipaddr, spki, fp);
        fclose(fp);
    }

    return TRUE;
}



/**
int  save_spki_with_ipaddr(Buffer ipa, Buffer pki, FILE* fp)

サーバの公開鍵情報（X.509の SubjectPublicKeyInfo）を識別子ipa と共にファイルに保存する．

識別子には通常は IPアドレスが使用される．@n
識別子，鍵情報の順で保存される．既にファイルが存在する場合はデータはファイルの最後に追加される．@n
識別子にIPアドレスを使用することにより，クライアント側でのサーバ認証に使用する．

@param  ipa   識別子．通常はサーバのIPアドレス．
@param  pki   保存する鍵情報（DER）．
@param  fp    保存先のファイル記述子

@retval TRUE  成功．
@retval FALSE 失敗．
*/
int  save_spki_with_ipaddr(Buffer ipa, Buffer pki, FILE* fp)
{    
    unsigned int  md;

    if (fp==NULL) return FALSE;

    md = FBRTL_IPADDRESS | FBRTL_STRING;
    if (!save_tagged_Buffer(ipa, fp, md, FALSE)) return FALSE;

    md = FBRTL_SPKI | FBRTL_ORIGINAL;
    if (!save_tagged_Buffer(pki, fp, md, FALSE)) return FALSE;

    return  TRUE;
}



/**
Buffer  read_spki_with_ipaddr(Buffer ipa, FILE* fp)

IPアドレス ipaを持つサーバの公開鍵情報（X.509の SubjectPublicKeyInfo）をファイルから読み込む．

@param  ipa  検索するIPアドレス
@param  fp   読み込むファイル記述子

@return IPアドレスipa の鍵情報(DER)．
*/
Buffer  read_spki_with_ipaddr(Buffer ipa, FILE* fp)
{
    unsigned int  md;
    Buffer  ips, pki;

    pki = init_Buffer();
    if (ipa.buf==NULL) return pki;

    md  = FBRTL_IPADDRESS | FBRTL_STRING;

    ips = read_tagged_Buffer(fp, &md);
    while (!feof(fp) && strcmp_Buffer(ipa, ips)) {
        ips = read_tagged_Buffer(fp, &md);
    }
        
    if (!strcmp_Buffer(ipa, ips)) {
        md  = FBRTL_SPKI | FBRTL_ORIGINAL;    
        pki = read_tagged_Buffer(fp, &md);
    }
    free_Buffer(&ips);

    return pki;
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////
// EVP API
//

/**
EVP_CIPHER* init_EVPAPI_Buffer(int type)

共通鍵暗号のアルゴリズムを指定する．

@param  type   暗号化オブジェクト（暗号化手法）
 現在サポートしているのは SSL_AES128CBC, SSL_3DES3CBC のみ

@return  暗号化方式．要 free()

OpenSSLの暗号化オブジェクトの種類（一部）．
@code
- AES:      EVP_aes_#_X()  #は 128, 192, 256  Xは ecb, cbc, cfb, ofb  例：EVP_aes_128_cbc()
- Blowfish: EVP_bf_X()  Xは ecb, cbc, cfb, ofb
- DES:      EVP_des_X() Xは ecb, cbc, cfb, ofb
- 3DES:     EVP_des_ede(),  EVP_des_ede_X()    Xは cbc, cfb, ofb
            EVP_des_ede3(), EVP_des_ede3_X()   Xは cbc, cfb, ofb
- RC4:      EVP_rc4(), EVP_rc4_40()
@endcode
*/
EVP_CIPHER* init_EVPAPI_Buffer(int type)
{
    EVP_CIPHER* cipher = NULL;

    if (type==SSL_AES128CBC) {
        cipher = (EVP_CIPHER*)EVP_aes_128_cbc();
    }
    else if (type==SSL_3DES3CBC) {
        cipher = (EVP_CIPHER*)EVP_des_ede3_cbc();
    }

    return cipher;
}



/**
Buffer  decode_EVPAPI_Buffer(Buffer buf, Buffer shdkey, EVP_CIPHER* cipher)

init_EVPAPI_Buffer() によって指定された方法により 暗号文bufを復号する．

@param  buf     復号する暗号データ
@param  shdkey  共有キー
@param  cipher  暗号方式
@return 復号文（平文）
*/
Buffer  decode_EVPAPI_Buffer(Buffer buf, Buffer shdkey, EVP_CIPHER* cipher)
{
    int  sz, ss;
    Buffer dec;
    EVP_CIPHER_CTX* ctx;
    unsigned char*  iv;

    dec = init_Buffer();
    if (shdkey.buf==NULL || cipher==NULL) return dec;

    iv  = (unsigned char*)&(shdkey.buf[shdkey.vldsz - SSL_IV_SIZE]);

#if OPENSSL_VERSION_NUMBER < 0x10101000L
    ctx = (EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX));  // v1.1.0
    EVP_CIPHER_CTX_init(ctx);
#else
    ctx = EVP_CIPHER_CTX_new();                             // v1.1.1
    EVP_CIPHER_CTX_reset(ctx);
#endif
    
    EVP_DecryptInit_ex(ctx, cipher, NULL, shdkey.buf, iv);

    dec = make_Buffer(buf.vldsz + EVP_CIPHER_CTX_block_size(ctx) + 1);
    if (dec.buf==NULL) return dec;

    EVP_DecryptUpdate(ctx, dec.buf, &sz, buf.buf, buf.vldsz);
    EVP_DecryptFinal_ex(ctx, &(dec.buf[sz]), &ss);
    dec.vldsz = sz + ss;
    dec.buf[dec.vldsz] = '\0';

#if OPENSSL_VERSION_NUMBER < 0x10101000L
    EVP_CIPHER_CTX_cleanup(ctx);
#endif
    free(ctx);
    return dec;
}



/**
Buffer  encode_EVPAPI_Buffer(Buffer buf, Buffer shdkey, EVP_CIPHER* cipher)

init_EVPAPI_Buffer() によって指定された方法により buf を暗号化する．

鍵は @b CRYPT_SharedKey を使う．
暗号化は @b SSL_ENC_BLCKSZ Byte毎に，暗号化される（ブロック暗号）

@param  buf     暗号化するデータ
@param  shdkey  共有キー
@param  cipher  暗号方式
@return 暗号文
*/
Buffer  encode_EVPAPI_Buffer(Buffer buf, Buffer shdkey, EVP_CIPHER* cipher)
{
    int  i, len, ss=0, sz;
    Buffer enc;
    EVP_CIPHER_CTX* ctx;
    unsigned char*  iv;

    enc = init_Buffer();
    //if (CRYPT_SharedKey==NULL || CRYPT_Type==NULL) return enc;
    if (shdkey.buf==NULL || cipher==NULL) return enc;

    iv  = (unsigned char*)&(shdkey.buf[shdkey.vldsz - SSL_IV_SIZE]);

#if OPENSSL_VERSION_NUMBER < 0x10101000L
    ctx = (EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX));  // v1.1.0
    EVP_CIPHER_CTX_init(ctx);
#else
    ctx = EVP_CIPHER_CTX_new();                             // v1.1.1
    EVP_CIPHER_CTX_reset(ctx);
#endif

    EVP_EncryptInit_ex(ctx, cipher, NULL, shdkey.buf, iv);

    len = buf.vldsz;
    enc = make_Buffer(len + EVP_CIPHER_CTX_block_size(ctx));
    if (enc.buf==NULL) return enc;

    for (i=0; i<len/SSL_ENC_BLCKSZ; i++) {
        EVP_EncryptUpdate(ctx, &(enc.buf[ss]), &sz, &(buf.buf[i*SSL_ENC_BLCKSZ]), SSL_ENC_BLCKSZ);
        ss += sz;
    }
    if (len%SSL_ENC_BLCKSZ!=0) {
        EVP_EncryptUpdate(ctx, &(enc.buf[ss]), &sz, &(buf.buf[i*SSL_ENC_BLCKSZ]), len%SSL_ENC_BLCKSZ);
        ss += sz;
    }
    EVP_EncryptFinal_ex(ctx, &(enc.buf[ss]), &sz);
    enc.vldsz = ss + sz;

#if OPENSSL_VERSION_NUMBER < 0x10101000L
    EVP_CIPHER_CTX_cleanup(ctx);
#endif
    free(ctx);
    //EVP_CIPHER_CTX_free(ctx);
    return  enc;
}



/**
void   free_EVP_CIPHER(EVP_CIPHER** p_cipher)
*/
void   free_EVP_CIPHER(EVP_CIPHER** p_cipher)
{
    if (p_cipher==NULL || *p_cipher==NULL) return;

    //free(*p_cipher);    // EVP_CIPHER* は解放する必要がない？
    *p_cipher = NULL;

    return;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Diffie-Hellman 鍵交換法
//

/**
int  save_DHspki_with_private(Buffer pki, FILE* fp)

Diffie-Hellman の公開鍵情報（X.509の SubjectPublicKeyInfo）pkiとプライベート鍵
(dhkeyより取得)をファイルに保存する．公開鍵情報，プライベート鍵の順で書き込まれる．
既にファイルが存在する場合，その内容は上書きされる．

@param  pki   保存する鍵情報（DER）．
@param  fp    ファイルポインタ 
@param  dhkey DH鍵．

@retval TRUE  成功．
@retval FALSE 失敗．ファイルの内容は保証されない．
*/
int  save_DHspki_with_private(Buffer pki, FILE* fp, DH* dhkey)
{    
    unsigned int  md;
    Buffer pv;

    if (fp==NULL || dhkey==NULL) return FALSE;

    md = FBRTL_SPKI | FBRTL_ORIGINAL;
    if (!save_tagged_Buffer(pki, fp, md, FALSE)) return FALSE;

    pv = get_DHprivatekey(dhkey);
    if (pv.buf==NULL) return FALSE;

    md = FBRTL_PRIV_KEY | FBRTL_ORIGINAL;
    if (!save_tagged_Buffer(pv,  fp, md, FALSE)) return FALSE;
    free_Buffer(&pv);

    return  TRUE;
}



/**
Buffer  read_DHspki_with_private(FILE* fp, DH** p_dhkey)

Diffie-Hellman の公開鍵情報（X.509の SubjectPublicKeyInfo）とプライペート鍵をファイルから読み込む．

@param  fp      ファイルポインタ 
@param  p_dhkey DHの鍵パラメータが返る．要 DH_free().
@return 読み込んだ鍵情報．
*/
Buffer  read_DHspki_with_private(FILE* fp, DH** p_dhkey)
{
    int  n = 0, code;
    unsigned int  md;
    Buffer  pp, pv, pk, gk, yk;

    pp = init_Buffer();
    if (fp==NULL) return pp;

    md = FBRTL_SPKI | FBRTL_ORIGINAL;
    pp = read_tagged_Buffer(fp, &md);
    if (pp.buf==NULL) return pp;

    if (*p_dhkey!=NULL) DH_free(*p_dhkey);
    *p_dhkey = DH_new();
    if (*p_dhkey==NULL) {
        free_Buffer(&pp);
        return pp;
    }
    
    pk = get_DHPkey(pp);
    gk = get_DHGkey(pp);
    yk = get_DHYkey(pp);

#if OPENSSL_VERSION_NUMBER < 0x10101000L
    // v1.1.0
    (*p_dhkey)->p = BN_bin2bn((const unsigned char*)(pk.buf), pk.vldsz, NULL);
    (*p_dhkey)->g = BN_bin2bn((const unsigned char*)(gk.buf), gk.vldsz, NULL);
    (*p_dhkey)->pub_key = BN_bin2bn((const unsigned char*)(yk.buf), yk.vldsz, NULL);
#else
    // v1.1.1
    BIGNUM* dhp_bn = BN_bin2bn((const unsigned char*)(pk.buf), pk.vldsz, NULL);
    BIGNUM* dhg_bn = BN_bin2bn((const unsigned char*)(gk.buf), gk.vldsz, NULL);
    BIGNUM* dhy_bn = BN_bin2bn((const unsigned char*)(gk.buf), gk.vldsz, NULL);

    if (dhp_bn!=NULL && dhg_bn!=NULL && dhy_bn!=NULL) {
        DH_set0_pqg(*p_dhkey, dhp_bn, NULL, dhg_bn);
        DH_set0_key(*p_dhkey, dhy_bn, NULL);
    }
    else {
        DH_free(*p_dhkey);
        *p_dhkey = NULL;
    }
#endif

    free_Buffer(&pk);
    free_Buffer(&gk);
    free_Buffer(&yk);

    // 鍵のチェック
    if (*p_dhkey!=NULL) n = DH_check(*p_dhkey, &code);
    if (n!=1 || code!=0) {
        if (*p_dhkey!=NULL) DH_free(*p_dhkey);
        free_Buffer(&pp);
        return pp;
    }

    // Private KEY の読み込み
    md = FBRTL_PRIV_KEY | FBRTL_ORIGINAL;
    pv = read_tagged_Buffer(fp, &md);
    if (pv.buf==NULL) {
        if (*p_dhkey!=NULL) DH_free(*p_dhkey);
        free_Buffer(&pp);
        return pp;
    }

#if OPENSSL_VERSION_NUMBER < 0x10101000L
    (*p_dhkey)->priv_key = BN_bin2bn((const unsigned char*)(pv.buf), pv.vldsz, NULL);
#else
    BIGNUM* priv_key = BN_bin2bn((const unsigned char*)(pv.buf), pv.vldsz, NULL);
    DH_set0_key(*p_dhkey, NULL, priv_key);
#endif

    free_Buffer(&pv);

    return  pp;
}



/**
Buffer  get_DHspki_ff(char* filename, int ks, DH** p_dhkey)

Generate DH SPKI from File.

ファイルから Diffie-Hellman の鍵を読み込む．

ただし以下の場合にはksのサイズの鍵を新たに生成し，ファイルに保存してから返す．
-# ファイルが存在しないか，鍵を読み込めない
-# ファイルにプライベート鍵が保存されていない
-# ファイルの中の鍵の長さが ksより短い

p_dhkey に DHの鍵パラメータが返る．

@param  filename  鍵が保存されているファイル
@param  ks        新たに生成する場合の鍵長(Bit)．
@param  p_dhkey   DHの鍵パラメータが返る．要 DH_free().

@return DER形式の公開鍵情報．
*/
Buffer  get_DHspki_ff(char* filename, int ks, DH** p_dhkey)
{
    Buffer  pki;
    FILE*   fp;

    pki = init_Buffer();
    if (filename==NULL || p_dhkey==NULL) return pki;

    if (file_exist(filename)) {
        //DEBUG_MODE PRINT_MESG("Load DH public key. ");
        fp = fopen(filename, "rb");
        pki = read_DHspki_with_private(fp, p_dhkey);
        fclose(fp);
        //DEBUG_MODE PRINT_MESG("... done.\n");
        if (pki.buf!=NULL) {
#if OPENSSL_VERSION_NUMBER < 0x10101000L
            if (DH_size(*p_dhkey)<(ks+7)/8 || (*p_dhkey)->priv_key==NULL) free_Buffer(&pki);    // v1.1.0
#else
            const BIGNUM* priv_key = DH_get0_priv_key(*p_dhkey);
            if (DH_size(*p_dhkey)<(ks+7)/8 || priv_key==NULL) free_Buffer(&pki);              // v1.1.0
#endif
        }
    }

    if (pki.buf==NULL) {
        //DEBUG_MODE PRINT_MESG("Generate DH public key.");
        pki = gen_DHspki(ks, p_dhkey);
        //DEBUG_MODE PRINT_MESG("... done.\n");

        fp = file_chmod_open(filename, (char*)"w", S_IRUSR | S_IWUSR);
        if (fp!=NULL) {
            save_DHspki_with_private(pki, fp, *p_dhkey);
            fclose(fp);
        }
    }

    /*
    DEBUG_MODE {
        Buffer enc;
        enc = encode_base64_Buffer(pki);
        DEBUG_MESG("SPKI = [%s]\n", enc.buf);
        free_Buffer(&enc);
    }*/
    return pki;
}



/**
Buffer  gen_DHspki(int ks, DH** p_dhkey)

Diffie-Hellman の鍵を生成し,公開鍵の完全な情報（X.509の SubjectPublicKeyInfo）のDER形式で返す．@n
p_dhkey に DHの鍵パラメータが返る．

@param  ks      鍵長(Bit)．512/1024/2048 を指定．
@param  p_dhkey DHの鍵パラメータが返る．要 DH_free().
@return DER形式の公開鍵情報．
*/
Buffer  gen_DHspki(int ks, DH** p_dhkey)
{
    int  sz, n, code;
    Buffer px, pp, pk;

    pk = init_Buffer();
    if (p_dhkey==NULL) return pk;

    //if (!RAND_load_file("/dev/random", 1024)) return pk;
    //DEBUG_MODE PRINT_MESG("Load /dev/urandom.\n");
    if (!RAND_load_file("/dev/urandom", 1024)) return pk;

    //DEBUG_MODE PRINT_MESG("Generate parameters.\n");
    do {
        if (*p_dhkey!=NULL) DH_free(*p_dhkey);
        *p_dhkey = DH_new();

#if OPENSSL_VERSION_NUMBER < 0x10101000L
        *p_dhkey = DH_generate_parameters(ks, DH_GENERATOR_2, NULL, NULL);
#else
        n = DH_generate_parameters_ex(*p_dhkey, ks, DH_GENERATOR_2, NULL);
#endif
        n = DH_check(*p_dhkey, &code);
    } while (n!=1 || code!=0);

    //DEBUG_MODE PRINT_MESG("Generate key.\n");
    sz = DH_generate_key(*p_dhkey);                 // 公開鍵(DH->pub_key)と秘密鍵(DH->priv_key)の生成
    if (sz==0) {
        DH_free(*p_dhkey);
        *p_dhkey = NULL;
        return pk;
    }

    //DEBUG_MODE PRINT_MESG("Check key size.\n");
    sz = i2d_DHparams(*p_dhkey, NULL);              // パラメタのサイズを検査 
    pp = px = make_Buffer(sz);                      // パラメタを入れるメモリを確保 
    if (pp.buf==NULL) {
        DH_free(*p_dhkey);
        *p_dhkey = NULL;
        return pk;
    }
    pp.vldsz = i2d_DHparams(*p_dhkey, &(px.buf));   // パラメタ(P,G鍵)を DER形式へ．pp.bufに格納． 
                                                    // px.bufは破壊される．

#if OPENSSL_VERSION_NUMBER < 0x10101000L
    sz = BN_num_bytes((*p_dhkey)->pub_key);
#else
    const BIGNUM* pub_key = DH_get0_pub_key(*p_dhkey);
    sz = BN_num_bytes(pub_key);
#endif

    px = make_Buffer(sz);
    if (px.buf==NULL) {
        DH_free(*p_dhkey);
        *p_dhkey = NULL;
        free_Buffer(&pp);
        return pk;
    }

#if OPENSSL_VERSION_NUMBER < 0x10101000L
    px.vldsz = BN_bn2bin((*p_dhkey)->pub_key, px.buf);
#else
    px.vldsz = BN_bn2bin(pub_key, px.buf);
#endif

    pk = join_DHpubkey(pp, px);                    // pp -> DHパラメタ(P,G鍵),  px-> Y鍵

    free_Buffer(&pp);
    free_Buffer(&px);

    return pk;
}



/**
Buffer  gen_DHspki_fs(Buffer pki, DH** p_dhkey)

Generate DH SPKI from Server's SPKI.

サーバの公開鍵の情報（X.509の SubjectPublicKeyInfo, DER形式）の中の P,G鍵から
自分の公開鍵情報（X.509 SPKI, DER形式）を作り出す．@n

@param  pki     相手の公開鍵情報（DER形式）
@param  p_dhkey DHの鍵パラメータが返る．要 DH_free().
@return 自分の公開鍵の情報（X.509の SubjectPublicKeyInfo, DER形式）
*/
Buffer  gen_DHspki_fs(Buffer pki, DH** p_dhkey)
{
    int  sz, n = 0, code;
    Buffer px, pp, pk;
    Buffer pkey, gkey;

    //
    pk = init_Buffer();
    if (p_dhkey==NULL) return pk;
    //
    if (*p_dhkey!=NULL) DH_free(*p_dhkey);
    *p_dhkey = DH_new();
    if (*p_dhkey==NULL) return pk;

    pkey = get_DHPkey(pki);
    gkey = get_DHGkey(pki);
    if (pkey.buf==NULL || gkey.buf==NULL) {
        free_Buffer(&pkey);
        free_Buffer(&gkey);
        DH_free(*p_dhkey);
        *p_dhkey = NULL;
        return pk;
    }

#if OPENSSL_VERSION_NUMBER < 0x10101000L
    (*p_dhkey)->p = BN_bin2bn((const unsigned char*)(pkey.buf), pkey.vldsz, NULL);
    (*p_dhkey)->g = BN_bin2bn((const unsigned char*)(gkey.buf), gkey.vldsz, NULL);
#else
    BIGNUM* dhp_bn = BN_bin2bn((const unsigned char*)(pkey.buf), pkey.vldsz, NULL);
    BIGNUM* dhg_bn = BN_bin2bn((const unsigned char*)(gkey.buf), gkey.vldsz, NULL);
    //
    if (dhp_bn!=NULL && dhg_bn!=NULL) {
        DH_set0_pqg(*p_dhkey, dhp_bn, NULL, dhg_bn);
    }
    else {
        DH_free(*p_dhkey);
        *p_dhkey = NULL;
    }
    //BN_free(dhp_bn);  // DHkeyの中身も消えてしまうので ここでは freeしない
    //BN_free(dhg_bn);
#endif

    free_Buffer(&pkey);
    free_Buffer(&gkey);

    //
    if (*p_dhkey!=NULL) n = DH_check(*p_dhkey, &code);
    if (n!=1 || code!=0) {
        if (*p_dhkey!=NULL) DH_free(*p_dhkey);
        *p_dhkey = NULL;
        return pk;
    }

    sz = DH_generate_key(*p_dhkey);
    if (sz==0) {
        DH_free(*p_dhkey);
        *p_dhkey = NULL;
        return pk;
    }

    //
    sz = i2d_DHparams(*p_dhkey, NULL);                  // パラメタのサイズを検査 
    pp = px = make_Buffer(sz);                          // パラメタを入れるメモリを確保 
    if (pp.buf==NULL) {
        DH_free(*p_dhkey);
        *p_dhkey = NULL;
        return pk;
    }
    pp.vldsz = i2d_DHparams(*p_dhkey, &(px.buf));       // パラメタ(P,G鍵)を DER形式へ．pp.bufに格納． 
                                                        // px.bufは破壊される．
#if OPENSSL_VERSION_NUMBER < 0x10101000L
    sz = BN_num_bytes((*p_dhkey)->pub_key);
#else
    const BIGNUM* pub_key = DH_get0_pub_key(*p_dhkey);
    sz = BN_num_bytes(pub_key);
#endif

    px = make_Buffer(sz);
    if (px.buf==NULL) {
        DH_free(*p_dhkey);
        *p_dhkey = NULL;
        free_Buffer(&pp);
        return px;
    }

#if OPENSSL_VERSION_NUMBER < 0x10101000L
    px.vldsz = BN_bn2bin((*p_dhkey)->pub_key, px.buf);  // 公開鍵(Y鍵)の DER形式 
#else
    px.vldsz = BN_bn2bin(pub_key, px.buf);              // 公開鍵(Y鍵)の DER形式 
#endif

    pk = join_DHpubkey(pp, px);                         // pp -> DHパラメタ(P,G鍵),  px-> Y鍵

    free_Buffer(&pp);
    free_Buffer(&px);

    return pk;
}



/**
Buffer get_DHsharedkey(Buffer pki, DH* dhkey)

相手の公開鍵の情報（X.509の SubjectPublicKeyInfo, DER形式）とDHkey（自分の鍵情報）を元に共通かぎを作り出す．

dhkkey には DH_generate_key(DHkey)（gen_DHspki() または gen_DHspki_fs()でも可）によって全てのパラメータ
（P,G,Y,秘密鍵）が設定されていなければならない． 

@param  pki    相手の公開鍵情報
@param  dhkey  自分の DH鍵
@return 共通鍵（バイナリ）

参考：man -M /usr/local/ssl/man bn
*/
Buffer get_DHsharedkey(Buffer pki, DH* dhkey)
{
    Buffer  ykey, skey;
    
    skey = init_Buffer();
    ykey = get_DHYkey(pki);
    if (ykey.buf==NULL) return skey;

    skey = get_DHsharedkey_fY(ykey, dhkey);
    
    free_Buffer(&ykey);
    return skey;
}



/**
Buffer  get_DHsharedkey_fY(Buffer ykey, DH* dhkey)

相手の（Diffie-Hellman の）Y鍵とDH Key（自分の鍵情報）を元に共通鍵を作り出す．

dhkkey には DH_generate_key(DHkey)（gen_DHspki() または gen_DHspki_fs()でも可）によって全てのパラメータ
（P,G,Y,秘密鍵）が設定されていなければならない． 

@param  ykey   相手の Y鍵（公開鍵）
@param  dhkey  自分の DH鍵
@return 共通鍵（バイナリ）

参考：man -M /usr/local/ssl/man bn
*/
Buffer  get_DHsharedkey_fY(Buffer ykey, DH* dhkey)
{
    int sz;
    Buffer  buf;

    buf = init_Buffer();
    if (dhkey==NULL) return buf;

    BIGNUM* yk = BN_bin2bn((const unsigned char*)(ykey.buf), ykey.vldsz, NULL);

    sz  = DH_size(dhkey);
    buf = make_Buffer(sz);
    buf.vldsz = sz;

    BN_free(yk);
    return buf;
}



/**
Buffer  get_DHYkey(Buffer param)

X.509の SubjectPublicKeyInfo のDER形式から，Diffie-Hellman のY鍵を取り出す． 

@param  param  X.509の SubjectPublicKeyInfo のDER形式
@return Y鍵（バイナリ）

参考：
@code
    [SEQ  [SEQ  [OBJ  Algor]
                [SEQ  [INT  P-key]
                      [INT  G-key]
                      [INT  P-keysize - 1]
                ]
          ]
          [BIT  Seed(0x00)  [INT  Y-key]]
    ]
@endcode
*/
Buffer  get_DHYkey(Buffer param)
{
    int     i, lp, sz=0;
    Buffer  pp;
    pp = init_Buffer();

    sz = skip_DER_node(param, JBXL_ASN1_SEQ_CNSTRCTD, sz, &lp);
    if (sz<0) return pp;

    sz = skip_DER_node(param, JBXL_ASN1_SEQ_CNSTRCTD, sz, &lp);
    if (sz<0) return pp;
    sz = sz + lp;

    sz = skip_DER_node(param, JBXL_ASN1_BIT, sz, &lp);
    if (sz<0) return pp;
    sz++;                        // for Seed(0x00)

    sz = skip_DER_node(param, JBXL_ASN1_INT, sz, &lp);
    if (sz<0) return pp;

    pp = make_Buffer(lp);
    if (pp.buf==NULL) return pp;
    for (i=0; i<lp; i++) pp.buf[i] = param.buf[sz+i];
    pp.vldsz = lp;
    return pp;
}



/**
Buffer  get_DHPkey(Buffer param)

X.509の SubjectPublicKeyInfo のDER形式から，Diffie-Hellman のP鍵を取り出す． 

@param  param  X.509の SubjectPublicKeyInfo のDER形式
@return P鍵（バイナリ）

参考：
@code
    [SEQ  [SEQ  [OBJ  Algor]
                [SEQ  [INT  P-key]
                      [INT  G-key]
                      [INT  P-keysize - 1]
                ]
          ]
          [BIT  Seed(0x00)  [INT  Y-key]]
    ]
@endcode
*/
Buffer  get_DHPkey(Buffer param)
{
    int     i, lp, sz=0;
    Buffer  pp;
    pp = init_Buffer();

    sz = skip_DER_node(param, JBXL_ASN1_SEQ_CNSTRCTD, sz, &lp);
    if (sz<0) return pp;

    sz = skip_DER_node(param, JBXL_ASN1_SEQ_CNSTRCTD, sz, &lp);
    if (sz<0) return pp;

    sz = skip_DER_node(param, JBXL_ASN1_OBJ, sz, &lp);
    if (sz<0) return pp;
    sz = sz + lp;

    sz = skip_DER_node(param, JBXL_ASN1_SEQ_CNSTRCTD, sz, &lp);
    if (sz<0) return pp;

    sz = skip_DER_node(param, JBXL_ASN1_INT, sz, &lp);
    if (sz<0) return pp;

    pp = make_Buffer(lp);
    if (pp.buf==NULL) return pp;
    for (i=0; i<lp; i++) pp.buf[i] = param.buf[sz+i];
    pp.vldsz = lp;
    return pp;
}



/**
Buffer  get_DHGkey(Buffer param)

X.509の SubjectPublicKeyInfo のDER形式から，Diffie-Hellman のG鍵を取り出す． 
普通は 2 か 5．

@param  param  X.509の SubjectPublicKeyInfo のDER形式
@return G鍵（バイナリ）

参考：
@code
    [SEQ  [SEQ  [OBJ  Algor]
                [SEQ  [INT  P-key]
                      [INT  G-key]
                      [INT  P-keysize - 1]
                ]
          ]
          [BIT  Seed(0x00)  [INT  Y-key]]
    ] 
@endcode
*/
Buffer  get_DHGkey(Buffer param)
{
    int     i, lp, sz=0;
    Buffer  pp;
    pp = init_Buffer();

    sz = skip_DER_node(param, JBXL_ASN1_SEQ_CNSTRCTD, sz, &lp);
    if (sz<0) return pp;

    sz = skip_DER_node(param, JBXL_ASN1_SEQ_CNSTRCTD, sz, &lp);
    if (sz<0) return pp;

    sz = skip_DER_node(param, JBXL_ASN1_OBJ, sz, &lp);
    if (sz<0) return pp;
    sz = sz + lp;

    sz = skip_DER_node(param, JBXL_ASN1_SEQ_CNSTRCTD, sz, &lp);
    if (sz<0) return pp;

    sz = skip_DER_node(param, JBXL_ASN1_INT, sz, &lp);
    if (sz<0) return pp;
    sz = sz + lp;

    sz = skip_DER_node(param, JBXL_ASN1_INT, sz, &lp);
    if (sz<0) return pp;

    pp = make_Buffer(lp);
    if (pp.buf==NULL) return pp;
    for (i=0; i<lp; i++) pp.buf[i] = param.buf[sz+i];
    pp.vldsz = lp;
    return pp;
}



/**
Buffer  get_DHalgorism(Buffer param)

X.509の SubjectPublicKeyInfo のDER形式から，アルゴリズム(Algor)を取り出す． 

@param  param  X.509の SubjectPublicKeyInfo のDER形式
@return アルゴリズム（バイナリ）

参考：
@code
    [SEQ  [SEQ  [OBJ  Algor]
                [SEQ  [INT  P-key]
                      [INT  G-key]
                      [INT  P-keysize - 1]
                ]
          ]
          [BIT  Seed(0x00)  [INT  Y-key]]
    ]  
@endcode
*/
Buffer  get_DHalgorism(Buffer param)
{
    int     i, lp, sz=0;
    Buffer  pp;
    pp = init_Buffer();

    sz = skip_DER_node(param, JBXL_ASN1_SEQ_CNSTRCTD, sz, &lp);
    if (sz<0) return pp;

    sz = skip_DER_node(param, JBXL_ASN1_SEQ_CNSTRCTD, sz, &lp);
    if (sz<0) return pp;

    sz = skip_DER_node(param, JBXL_ASN1_OBJ, sz, &lp);
    if (sz<0) return pp;

    pp = make_Buffer(lp);
    if (pp.buf==NULL) return pp;
    for (i=0; i<lp; i++) pp.buf[i] = param.buf[sz+i];
    pp.vldsz = lp;
    return pp;
}


/**
Buffer  get_DHprivatekey(DH* dhkey)

DH鍵 dh からプライベート鍵を取り出して返す．

@param  dhkey  DH鍵．
@return プライベート鍵
*/
Buffer  get_DHprivatekey(DH* dhkey)
{
    int    sz;
    Buffer pv;
 
#if OPENSSL_VERSION_NUMBER < 0x10101000L
    sz = BN_num_bytes(dhkey->priv_key);
//#elif OPENSSL_VERSION_NUMBER < 0x30000000L
#else
    const BIGNUM* priv_key = DH_get0_pub_key(dhkey);
    sz = BN_num_bytes(priv_key);
#endif

    pv = make_Buffer(sz);
    if (pv.buf==NULL) return pv;

#if OPENSSL_VERSION_NUMBER < 0x10101000L
    pv.vldsz = BN_bn2bin(dhkey->priv_key, pv.buf);
//#elif OPENSSL_VERSION_NUMBER < 0xF0000000L
#else
    pv.vldsz = BN_bn2bin(priv_key, pv.buf);
#endif

    return pv;
}



/**
Buffer  join_DHpubkey(Buffer param, Buffer key)

Diffie-Hellman の鍵交換のための X.509の SubjectPublicKeyInfo を取得する．
DHパラメタ(P+G)とDH鍵(Y)のDER形式を結合して,完全な鍵情報のDER形式をつくる．

@param  param  DHパラメータ(P,G鍵)．i2d_DHparams()による DER形式．
@param  key    DH公開鍵(Y鍵)の DER形式．
@return DER形式の公開鍵情報．[[Algor + [P + G + PKeySize]] + [BIT [Y]]]


参考： param のデータ形式は
@code
    [SEQ  [INT  P-key]
          [INT  G-key]
    ]
@endcode
である．これと Y-key から 下記のデータを生成する．
@code
    [SEQ  [SEQ  [OBJ  Algor]
                [SEQ  [INT  P-key]
                      [INT  G-key]
                      [INT  P-keysize - 1]
                ]  
          ]
          [BIT(未使用bit無し)  [INT  Y-key]]
    ]  
@endcode

@attention
DHのアルゴリズムは {0x06,0x07,0x2a,0x86,0x48,0xce,0x3e,0x02,0x01} だと思うのだが,RSAのアルゴリズムで動くようだ？
*/
Buffer  join_DHpubkey(Buffer param, Buffer key)
{
//    unsigned char dh_algor[]={0x06,0x07,0x2a,0x86,0x48,0xce,0x3e,0x02,0x01};            // DH  ?
    unsigned char dh_algor[]={0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x03,0x01};    // RSA ?
                                            // ↑ RSAアルゴリズム？ 
    int len_dh_algor = 11;

    int ls, lp, la, sz;
    Buffer  px, pp, pm;

    // [P鍵 + G鍵] → pp 
    pp = init_Buffer();
    sz = skip_DER_node(param, JBXL_ASN1_SEQ_CNSTRCTD, 0, &lp);
    if (sz<0) return pp;

    pp.buf   = param.buf   + sz;
    pp.bufsz = param.bufsz - sz;
    pp.vldsz = param.vldsz - sz;

    // [鍵サイズ] → pm 
    px = int2bin_DER(key.vldsz*8-1);
    pm = node2DER(px, JBXL_ASN1_INT);
    ls = pm.vldsz;
    free_Buffer(&px);

    // pp[P鍵 + G鍵] + pm[鍵サイズ] → pm 
    px = make_Buffer(lp+ls); 
    memcpy(px.buf,    pp.buf, lp);
    memcpy(px.buf+lp, pm.buf, ls);  
    px.vldsz = lp + ls;
    free_Buffer(&pm);              // ppを freeしてはいけない 
    pm = node2DER(px, JBXL_ASN1_SEQ_CNSTRCTD); 
    ls = pm.vldsz;
    free_Buffer(&px);

    // dh_algor[アルゴリズム] + pm[P鍵 + G鍵 + 鍵サイズ] → px 
    la = len_dh_algor; 
    pp = make_Buffer(ls+la); 
    memcpy(pp.buf,  dh_algor, la);
    memcpy(pp.buf+la, pm.buf, ls);
    pp.vldsz = ls + la;
    free_Buffer(&pm);         
    px = node2DER(pp, JBXL_ASN1_SEQ_CNSTRCTD);
    ls = px.vldsz;
    free_Buffer(&pp);  

    // px[アルゴリズム + P鍵 + G鍵 + 鍵サイズ] + pp[Y鍵] → pm 
    pm = node2DER(key, JBXL_ASN1_INT);
    pp = node2DER(pm,  JBXL_ASN1_BIT);
    lp = pp.vldsz;
    free_Buffer(&pm); 
    pm = make_Buffer(ls+lp);
    memcpy(pm.buf,    px.buf, ls);
    memcpy(pm.buf+ls, pp.buf, lp);
    pm.vldsz = ls + lp;
    free_Buffer(&px);
    free_Buffer(&pp);  

    pp = node2DER(pm, JBXL_ASN1_SEQ_CNSTRCTD);
    free_Buffer(&pm);
    return pp;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////
// SSL/TLS
//         Reference: http://h71000.www7.hp.com/doc/83final/BA554_90007/ch04s03.html
//                    http://x68000.q-e-d.net/~68user/net/
//

/**
<b>SSL/TLS 関数</b>

@attention
注：この関数群では，SSL_CTX* と SSL* の変数は一対一に対応している
効率は悪いが，ソースが見易くなる．
CTX: 証明書の取り扱い
*/


/**
void  ssl_init()

SSL/TLS の初期化を行う．これに先立って，乱数の初期化も行っておいた方が良い．
*/
void  ssl_init()
{
    SSL_library_init();
    SSL_load_error_strings();
}



/**
SSL_CTX*  ssl_client_setup(char* ca)

SSL/TLS のクライントソケット作成の前処理．@n
認証局証明書等を設定し，SSLコンテキストを返す．@n
SSL/TLSのクライントソケットを作成する前に，必ず実行しなければならない．

@param  ca       認証局の自己証明書 PEM形式．クライアント認証用．NULL可．

@retval NULL以外 SSLコンテキストへのポインタ．
@retval NULL     エラー．
*/
SSL_CTX*  ssl_client_setup(char* ca)
{
    SSL_CTX* ssl_ctx;

#if OPENSSL_VERSION_NUMBER < 0x10101000L
    ssl_ctx = SSL_CTX_new(SSLv23_client_method());
#else
    ssl_ctx = SSL_CTX_new(TLS_client_method());
#endif
    if (ssl_ctx==NULL) return NULL;

    // 認証局データを読み込む（PEMデータ）
    if (ca!=NULL) SSL_CTX_load_verify_locations(ssl_ctx, ca, NULL);

    return ssl_ctx;
}



/**
SSL*  ssl_client_socket(int sock, SSL_CTX* ssl_ctx, int mode)

SSL/TLS のストリームを作成し，ソケットと関連付ける．@n
これにより相手（SSLをしゃべるサーバ）とSSL/TLS通信が可能になる．

@param  sock     SSL通信を行うソケット．
@param  ssl_ctx  SSLコンテキストへのポインタ．
@param  mode     サーバ証明書のチェックを行うか？
@param  mode     @b ON:  行う．認証エラーの場合は NULL を返す．
@param  mode     @b OFF: ここでは行わない．ca が NULLの場合は強制的に OFFになる．

@retval NULL以外 SSL用ストリームへのポインタ．
@retval NULL     エラー．

@attention この関数によって取得した SSL* 変数は，必ず ssl_close() でクローズすること．
*/
SSL*  ssl_client_socket(int sock, SSL_CTX* ssl_ctx, int mode)
{
    if (ssl_ctx==NULL) return NULL;
    //
    SSL* ssl = SSL_new(ssl_ctx);
    if (ssl==NULL) {
        //SSL_CTX_free(ssl_ctx);
        return NULL;
    }

    int ret = SSL_set_fd(ssl, sock);
    if (ret!=1) {
        ssl_close(ssl);
        return NULL;
    }

    // サーバへ接続
    ret = SSL_connect(ssl);
    if (ret!=1) {
        ssl_close(ssl);
        return NULL;
    }

    // サーバ証明書のチェック
    if (mode==ON) {
        long lrt = SSL_get_verify_result(ssl);
        if (lrt!=X509_V_OK) {
            ssl_close(ssl);
            return NULL;
        }
    }

    return ssl;
}



/**
SSL_CTX*  ssl_server_setup(char* crt_fn, char* key_fn)

SSL/TLS のサーバソケット作成の前処理．@n
サーバ証明書等を設定し，SSLコンテキストを返す．@n
SSL/TLS のサーバソケットを作成する前に，必ず実行しなければならない．@n
中間証明書が必要な場合，crt_fn にフルチェーンのファイルを指定しても良い．
その場合は，chn_fn に NULL を指定する． 

@param  crt_fn  サーバ証明書のファイル名（PEM形式）
@param  key_fn  サーバの秘密鍵のファイル名（PEM形式）
@param  chn_fn  証明書チェーンのファイル名（PEM形式）

@return SSLコンテキストへのポインタ．エラーの場合は NULLが返る．
*/
SSL_CTX*  ssl_server_setup(char* crt_fn, char* key_fn, char* chn_fn)
{
    int  ret;
    SSL_CTX* ssl_ctx;

    //
#if OPENSSL_VERSION_NUMBER < 0x10101000L
    ssl_ctx = SSL_CTX_new(SSLv23_server_method());
#else
    ssl_ctx = SSL_CTX_new(TLS_server_method());
#endif
    if (ssl_ctx==NULL) return NULL;

    // サーバ証明書と秘密鍵の読み込み
    if (crt_fn!=NULL && key_fn!=NULL) {
        if (chn_fn!=NULL) {
            ret = SSL_CTX_use_certificate_file(ssl_ctx, crt_fn, SSL_FILETYPE_PEM);
        }
        else {
            ret = ssl_read_fullchain_cert_file(ssl_ctx, crt_fn);
        }

        if (ret==1) {
            ret = SSL_CTX_use_PrivateKey_file(ssl_ctx, key_fn, SSL_FILETYPE_PEM);
        }
        
        if (ret==1 && chn_fn!=NULL) {
            ret = ssl_add_chain_file(ssl_ctx, chn_fn);
        }

        if (ret!=1) {
            SSL_CTX_free(ssl_ctx);
            return NULL;
        }

        //SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, NULL);   // Client Certificate
        //SSL_CTX_set_verify_depth(ssl_ctx, 1);
    }

    return ssl_ctx;
}



/**
int  ssl_add_chain_file(SSL_CTX* ssl_ctx, char* file)

証明書チェーンのファイルからすべての中間証明書を読み込んで証明書ストアに追加する．

@param  ssl_ctx SSL_CTX へのポインタ
@param  file  証明書チェーンのファイル名（PEM形式）

@return 1: 正常終了
*/
int  ssl_add_chain_file(SSL_CTX* ssl_ctx, char* file)
{
    int ret = 0;
    if (file==NULL) return ret;

    BIO* bio = BIO_new(BIO_s_file());
    if (bio==NULL) return ret;

    BIO_read_filename(bio, file);
    X509* x509 = PEM_read_bio_X509(bio, NULL, NULL, NULL);
    while(x509!=NULL) {
        ret = SSL_CTX_add_extra_chain_cert(ssl_ctx, x509);
        x509 = PEM_read_bio_X509(bio, NULL, NULL, NULL);
    }
    BIO_free(bio);

    return ret;
}



/**
int  ssl_read_fullchain_cert_file(SSL_CTX* ssl_ctx, char* file)

フルチェーンのファイル（サーバ証明書と中間証明書を内包）から全ての証明書を読み込んで証明書ストアに保存する．

@param  ssl_ctx SSL_CTX へのポインタ
@param  file  フルチェーンの証明書のファイル名（PEM形式）

@return 1: 正常終了
*/
int  ssl_read_fullchain_cert_file(SSL_CTX* ssl_ctx, char* file)
{
    int ret = 0;
    if (file==NULL) return ret;

    BIO*  bio  = NULL;
    X509* x509 = NULL;    

    bio = BIO_new(BIO_s_file());
    if (bio==NULL) return ret;
    BIO_read_filename(bio, file);

    // cert
    x509 = PEM_read_bio_X509(bio, NULL, NULL, NULL);
    if (x509!=NULL) {
        ret = SSL_CTX_use_certificate(ssl_ctx, x509);
    }
    // chain
    if (ret==1) {
        x509 = PEM_read_bio_X509(bio, NULL, NULL, NULL);
        while(x509!=NULL) {
            ret = SSL_CTX_add_extra_chain_cert(ssl_ctx, x509);
            x509 = PEM_read_bio_X509(bio, NULL, NULL, NULL);
        }
    }

    BIO_free(bio);

    return ret;
}




#define  SSL_ACCEPT_COUNT_MAX  10

/**
SSL*  ssl_server_socket(int sock, SSL_CTX* ssl_ctx)

SSL/TLS のサーバ用TCPストリームを作成し，ソケットと関連付ける．@n
これに先立って TCPの接続を確立とサーバ証明書を設定して置かなければならない．

@param  sock    SSL通信を行うソケット．sock は accept() の戻り値を使用する．
@param  ssl_ctx SSLコンテキストへのポインタ．

@return SSL用ストリームへのポインタ．エラーの場合は NULLが返る．

@attention この関数によって取得した SSL* 変数は，ssl_close() でクローズすること．
*/
SSL*  ssl_server_socket(int sock, SSL_CTX* ssl_ctx)
{
    if (ssl_ctx==NULL) return NULL;
    int cnt_max = SSL_ACCEPT_COUNT_MAX;

    SSL* ssl = SSL_new(ssl_ctx);
    if (ssl==NULL) {
        //SSL_CTX_free(ssl_ctx);
        return NULL;
    }

    int ret = SSL_set_fd(ssl, sock);
    if (ret!=1) {
        ssl_close(ssl);
        return NULL;
    }

    int cnt = 0;
    SSL_set_accept_state(ssl);
    do {
        ret = SSL_accept(ssl);
        ret = SSL_get_error(ssl, ret);
        cnt++;
    } while (cnt<=cnt_max && (ret==SSL_ERROR_WANT_READ || ret==SSL_ERROR_WANT_WRITE || ret==SSL_ERROR_SYSCALL));
    
    if (ret==SSL_ERROR_SSL || cnt>cnt_max) {
        if (ret!=SSL_ERROR_SSL) ssl_close(ssl);
        return NULL;
    }

    return ssl;
}



/**
void  ssl_close(SSL* ssl)

SSLストリームをクローズし，使用した変数を開放する．

@param  ssl  クローズする SSL用ストリーム
*/
void  ssl_close(SSL* ssl)
{
    if (ssl!=NULL) {
        //SSL_CTX* p = ssl->ctx;
        SSL_shutdown(ssl);
        SSL_free(ssl);
        //if (p!=NULL) SSL_CTX_free(p);
    }
}



/**
tList*  ssl_get_cert_info(SSL* ssl)

SSLストリームから証明書の情報を得る．これに先立って 接続が connect していないといけない．

@param  ssl  SSL用ストリーム

@return SSLの情報を格納したリストへのポインタ．
        獲得できる情報は，cipher, version, subject, issuer, before, after
*/
tList*  ssl_get_cert_info(SSL* ssl)
{
    tList* lt;
    tList* lp;
    char* pp;
    char  buf[LBUF];
    X509* x509;
    BIO*  bio;

    pp = (char*)SSL_get_cipher(ssl);
    lt = lp = add_tList_node_str(NULL, "cipher", pp);

    pp = (char*)SSL_get_cipher_version(ssl);
    lp = add_tList_node_str(lp, "version", pp);

    x509 = SSL_get_peer_certificate(ssl); 
    if (x509==NULL) return lt;

    bio = BIO_new(BIO_s_mem());
    if (bio==NULL) {
        X509_free(x509);
        return lt;
    }

    X509_NAME_print_ex(bio, X509_get_subject_name(x509), 0, XN_FLAG_RFC2253);
    BIO_gets(bio, buf, LBUF);
    lp = add_tList_node_str(lp, "subject", buf);

    X509_NAME_print_ex(bio, X509_get_issuer_name(x509), 0, XN_FLAG_RFC2253);
    BIO_gets(bio, buf, LBUF);
    lp = add_tList_node_str(lp, "issuer", buf);

    ASN1_TIME_print(bio, X509_get_notBefore(x509));
    BIO_gets(bio, buf, LBUF);
    lp = add_tList_node_str(lp, "before", buf);

    ASN1_TIME_print(bio, X509_get_notAfter(x509));
    BIO_gets(bio, buf, LBUF);
    lp = add_tList_node_str(lp, "after", buf);

    BIO_free(bio);
    X509_free(x509);

    return lt;
}




/////////////////////////////////////////////////////////////////
// SSL communications

/** 
int  ssl_recv(SSL* ssl, char* rmsg, int size)
     
SSL_read()をラッピングした関数．SSL経由でデータを受信する．
     
@param  ssl    SSL用ストリーム
@param  rmsg   受信用データバッファ
@param  size   データバッファのサイズ
     
@retval 1以上  受信したバイト数．
@retval 0      おそらくは相手側がセッションをクローズした．
@retval JBXL_ARGS_ERROR      引数に NULLのデータがある．
@retval JBXL_SSL_RECV_ERROR  受信エラー．

@see tcp_recv() 
*/  
int  ssl_recv(SSL* ssl, char* rmsg, int size)
{
    int cc;

    if (ssl==NULL || rmsg==NULL) return JBXL_ARGS_ERROR;

    memset(rmsg, 0, size);
    cc = SSL_read(ssl, rmsg, size-1);
     
    if (cc<0) cc = JBXL_SSL_RECV_ERROR;
    return cc;
}



/**    
int  ssl_send(SSL* ssl, char* smsg, int size)
     
SSL_write()をラッピングした関数．SSL経由でデータを送る．

データ(smsg)のサイズ sizeに0以下を指定した場合は、smsgは文字列であると見なして,サイズを自動的に計算する．
     
@param  ssl    SSL用ストリーム
@param  smsg   送信するデータ
@param  size   送信するデータ（smsg）のサイズ．サイズが 0以下の場合は smsgは文字列であるとみなす．
     
@retval 0以上 送信バイト数．
@retval JBXL_ARGS_ERROR      引数に NULLのデータがある．
@retval JBXL_SSL_SEND_ERROR  送信エラー．

@see also tcp_send()
*/
int  ssl_send(SSL* ssl, char* smsg, int size)
{
    int cc;

    if (ssl==NULL || smsg==NULL) return JBXL_ARGS_ERROR;

    if (size<=0) size = strlen(smsg);
    cc = SSL_write(ssl, smsg, size);
    
    if (cc<0) cc = JBXL_SSL_SEND_ERROR;
    return cc;
}



/**    
int  ssl_recv_wait(int sock, SSL* ssl, char* mesg, int sz, int tm)
      
SSL経由でデータを受信する．

タイムアウトの設定が可能．タイムアウトに 0を指定した場合, recv_wait()
関数を呼び出した時点で読み込み可能データがなければすぐにタイムアウト
となる (JBXL_NET_RECV_TIMEOUTが返る)．
   
@param  sock    ソケット記述子
@param  ssl     SSL用ストリーム
@param  mesg    受信用データバッファ
@param  sz      データバッファのサイズ
@param  tm      タイムアウト時間．秒単位．
   
@retval 1以上  受信したバイト数．
@retval 0      おそらくは相手側がセッションをクローズした．
@retval JBXL_ARGS_ERROR       引数に NULLのデータがある．
@retval JBXL_SSL_RECV_ERROR   受信エラー．
@retval JBXL_NET_RECV_TIMEOUT タイムアウト．

@see tcp_recv_wait() 
*/
int  ssl_recv_wait(int sock, SSL* ssl, char* mesg, int sz, int tm)
{
    int  cc;
                 
    if (ssl==NULL || mesg==NULL) return JBXL_ARGS_ERROR;

    //memset(mesg, 0, sz);
    if (recv_wait(sock, tm)) {
        cc = ssl_recv(ssl, mesg, sz);
        //if (cc<=0) DEBUG_MODE PRINT_MESG("SSL_RECV_WAIT: Session Closed.\n");
    }
    else {
        //DEBUG_MODE PRINT_MESG("SSL_RECV_WAIT: Time Out. (%ds)\n", tm);
        return JBXL_NET_RECV_TIMEOUT;
    }
    
    return cc;
}



/**    
int  ssl_send_mesgln(SSL* ssl, char* mesg)
      
SSLメッセージ(文字列)に改行(@\r@\n)を付け加えて送信する．
   
@param  ssl    SSL用ストリーム
@param  mesg   送信用メッセージ
   
@retval 0以上 送信バイト数（改行を含む）．
@retval JBXL_ARGS_ERROR      引数に NULLのデータがある．
@retval JBXL_SSL_SEND_ERROR  送信失敗．
*/    
int  ssl_send_mesgln(SSL* ssl, char* mesg)
{  
    int   cc, sz;
    char* buf;
    
    if (ssl==NULL || mesg==NULL) return JBXL_ARGS_ERROR;

    sz = strlen(mesg) + 3;    // for CR+LF+0x00 
    buf = (char*)malloc(sz);
    if (buf==NULL) return JBXL_MALLOC_ERROR;
    
    strncpy(buf, mesg, sz);
    strncat(buf, "\r\n", 2);
    cc = ssl_send(ssl, buf, strlen(buf));
    
    free(buf);
    return cc;
}



/**
int  ssl_recv_mstream(int sock, SSL* ssl, char* mesg, int sz, mstream* sb, int tm)

SSL/TSL経由でメッセージを受信する．

受信メッセージはメッセージストリーム バッファに一旦バッファリングされ，この関数により一行ずつ読み出される．
mesgには最大 sz-1文字が格納される．もし，バッファ中の一行のデータが sz-1より大きい場合は，はみ出した部分は捨てられる．

mesgに格納される時，行中の改行コードは削除され，行末には必ず '@\0' が入る．
タイムアウトの設定が可能でタイムアウトに 0を指定した場合, 呼び出した時点で
読み込み可能データがなければすぐにタイムアウトとなる (JBXL_NET_RECV_TIMEOUT が返る)．
メッセージストリームのバッファ部が確保されていない場合は，最初に呼び出された時点で確保される．

一旦この関数を使用して，受信データをバッファリングしたら，ソケットをクローズするまで，
読み取りには必ず同じストリームを使用してこの関数を呼び出さばけ
ればならない．そうで無い場合は受信データの整合性は保証されない．

@param  sock    ソケット記述子
@param  ssl     SSL用ストリーム
@param  mesg    受信用データバッファ．予め十分なメモリ領域を確保しておく．
@param  sz      データバッファのサイズ
@param  sb      リングバッファ型のストリームバッファ．バッファ部が確保さえていなければ，自動的に確保される．
@param  tm      タイムアウト時間．秒単位．

@retval 1以上  mesgに格納したメッセージのバイト数．
@retval 0      おそらくは相手側がセッションをクローズした．
@retval JBXL_SSL_RECV_ERROR     受信エラー．
@retval JBXL_ARGS_ERROR         引数に NULLのデータがある．
@retval JBXL_MALLOC_ERROR       メッセージバッファ部が無いので確保しようとしたが，確保に失敗した．
@retval JBXL_NET_BUF_ERROR      メッセージバッファにデータは存在するはずだが，原因不明の理由により獲得に失敗した．
@retval JBXL_NET_BUFSZ_ERROR    メッセージバッファ中のデータの長さが，mesgの長さより大きい．はみ出した部分は捨てられた．
@retval JBXL_NET_RECV_TIMEOUT   タイムアウト．
*/
int  ssl_recv_mstream(int sock, SSL* ssl, char* mesg, int sz, mstream* sb, int tm)
{
    int  cc;
    unsigned char* pp;

    if (ssl==NULL) return JBXL_ARGS_ERROR;
    if (mesg==NULL || sb==NULL) return JBXL_ARGS_ERROR;
    //memset(mesg, 0, sz);

    if (sb->buf==NULL) {
        *sb = make_mstream(RECVBUFSZ);
        if (sb->buf==NULL) return JBXL_MALLOC_ERROR;
    }

    while (sb->datano==0) {
        cc = ssl_recv_wait(sock, ssl, mesg, sz, tm);
        if (cc<=0) return cc;
        put_mstream(sb, (unsigned char*)mesg);
        //memset(mesg, 0, sz);
    }

    pp = get_mstream(sb);
    if (pp==NULL) return JBXL_NET_BUF_ERROR;
    if (strlen((const char*)pp)>=(unsigned int)sz) {
        memcpy(mesg, pp, sz-1);
        free(pp);
        return JBXL_NET_BUFSZ_ERROR;
    }
    memcpy(mesg, pp, strlen((const char*)pp));

    free(pp);
    return strlen(mesg);
}




/////////////////////////////////////////////////////////////////
// SSL with Buffer

/**
int  ssl_recv_Buffer(SSL* ssl, Buffer* str)

SSL経由でデータを受信する．バッファリングなし．

@param  ssl     SSL用ストリーム
@param  str     受信用データバッファ．予めメモリを確保しておく．

@retval 1以上  受信したバイト数．
@retval 0      おそらくは相手側がセッションをクローズした．正常切断
@retval JBXL_ARGS_ERROR      引数に NULLのデータがある．
@retval JBXL_SSL_RECV_ERROR  受信エラー．
*/
int  ssl_recv_Buffer(SSL* ssl, Buffer* str)
{
    int cc;

    if (ssl==NULL || str==NULL || str->buf==NULL) return JBXL_ARGS_ERROR;

    memset(str->buf, 0, str->bufsz);
    str->vldsz = 0;
    str->state = JBXL_NORMAL;

    cc = SSL_read(ssl, str->buf, str->bufsz-1);
    if (cc>=0) str->vldsz = cc;
    else  cc = JBXL_SSL_RECV_ERROR;
    
    return cc;
}



/**
int  ssl_send_Buffer(SSL* ssl, Buffer* str)

SSL経由でデータを送信する．

@param  ssl    SSL用ストリーム
@param  str    送信用データバッファ．

@retval 0以上 送信バイト数．
@retval JBXL_ARGS_ERROR      引数に NULLのデータがある．
@retval JBXL_SSL_SEND_ERROR  送信失敗．
*/
int  ssl_send_Buffer(SSL* ssl, Buffer* str)
{
    int cc;

    if (ssl==NULL || str==NULL || str->buf==NULL) return JBXL_ARGS_ERROR;

    if (str->vldsz<0) str->vldsz = strlen((const char*)str->buf);
    cc = SSL_write(ssl, str->buf, str->vldsz);

    if (cc<0) cc = JBXL_SSL_SEND_ERROR;
    return cc;
}



/**
int  ssl_recv_Buffer_wait(int sock, SSL* ssl, Buffer* str, int tm)

SSL経由でデータを受信する．

タイムアウトの設定が可能．タイムアウトに 0を指定した場合, recv_wait()
関数を呼び出した時点で読み込み可能データがなければすぐにタイムアウト
となる (JBXL_NET_RECV_TIMEOUTが返る)．

@param  sock    ソケット記述子
@param  ssl     SSL用ストリーム
@param  str     受信用データバッファ．予めメモリを確保しておく．
@param  tm      タイムアウト時間．秒単位．

@retval 1以上  受信したバイト数．
@retval 0      おそらくは相手側がセッションをクローズした．
@retval JBXL_ARGS_ERROR        引数に NULLのデータがある．
@retval JBXL_SSL_RECV_ERROR    受信エラー．
@retval JBXL_NET_RECV_TIMEOUT  タイムアウト．
*/
int  ssl_recv_Buffer_wait(int sock, SSL* ssl, Buffer* str, int tm)
{
    int cc;

    if (ssl==NULL || str==NULL || str->buf==NULL) return JBXL_ARGS_ERROR;

    //memset(str->buf, 0, str->bufsz);
    str->vldsz = 0;
    cc = ssl_recv_wait(sock, ssl, (char*)str->buf, str->bufsz, tm);
    if (cc>=0) str->vldsz = cc;

    return cc;
}



/**
int  ssl_send_sBuffer(SSL* ssl, Buffer* str)

SSL経由で文字列データを送信する．

@param  ssl    SSL用ストリーム
@param  str    送信用メッセージバッファ．

@retval 0以上 送信バイト数．
@retval JBXL_ARGS_ERROR      引数に NULLのデータがある．
@retval JBXL_SSL_SEND_ERROR  送信失敗．
*/
int  ssl_send_sBuffer(SSL* ssl, Buffer* str)
{
    int cc;

    if (ssl==NULL || str==NULL || str->buf==NULL) return JBXL_ARGS_ERROR;

    cc = SSL_write(ssl, str->buf, strlen((const char*)str->buf));

    if (cc<0) cc = JBXL_SSL_SEND_ERROR;
    return cc;
}



/**
int  ssl_send_sBufferln(SSL* ssl, Buffer* str)

SSLメッセージ(文字列)に改行(@\r@\n)を付け加えて送信する．

@param  ssl    SSL用ストリーム
@param  str    送信用メッセージバッファ．

@retval 0以上 送信バイト数（改行を含む）．
@retval JBXL_ARGS_ERROR      引数に NULLのデータがある．
@retval JBXL_SSL_SEND_ERROR  送信失敗．
*/
int  ssl_send_sBufferln(SSL* ssl, Buffer* str)
{
    int    cc;
    Buffer buf;

    if (ssl==NULL || str==NULL || str->buf==NULL) return JBXL_ARGS_ERROR;

    buf = dup_Buffer(*str);
    cat_s2Buffer("\r\n", &buf);
    cc = SSL_write(ssl, buf.buf, strlen((const char*)buf.buf));
    free_Buffer(&buf);

    if (cc<0) cc = JBXL_SSL_SEND_ERROR;
    return cc;
}



/**
int  ssl_recv_mstream_Buffer(int sock, SSL* ssl, Buffer* mesg, mstream* sb, int tm)

SSL経由でメッセージを受信する．@n
受信メッセージはメッセージストリームに一旦バッファリングされ，この関数により一行ずつ読み出される．

mesgに格納される時，行中の改行コードは削除され，行末には必ず '@\0' が入る．
タイムアウトの設定が可能でタイムアウトに 0を指定した場合, 呼び出した時点で
読み込み可能データがなければすぐにタイムアウトとなる (JBXL_NET_RECV_TIMEOUT が返る)．
メッセージストリームのバッファ部が確保されていない場合は，最初に呼び出された時点で
確保される．

一旦この関数を使用して，受信データをバッファリングしたら，ソケットをクローズするまで，
読み取りには必ず同じストリームを使用してこの関数を呼び出さばければならない．
そうで無い場合は受信データの整合性は保証されない．

@param  sock    ソケット記述子
@param  ssl     SSL用ストリーム
@param  mesg    受信用データバッファ．バッファ部を確保する必要はない．
@param  sb      メッセージバッファ（リング型のストリームバッファ）．バッファ部が確保さえていなければ，自動的に確保される．
@param  tm      タイムアウト時間．秒単位．

@retval 1以上  mesgに格納したメッセージのバイト数．
@retval 0      おそらくは相手側がセッションをクローズした．
@retval JBXL_SSL_RECV_ERROR    受信エラー．
@retval JBXL_ARGS_ERROR        引数に NULLのデータがある．
@retval JBXL_MALLOC_ERROR      メッセージバッファ部が無いので確保しようとしたが，確保に失敗した．
@retval JBXL_NET_BUF_ERROR     メッセージバッファにデータは存在するはずだが，原因不明の理由により獲得に失敗した．
@retval JBXL_NET_RECV_TIMEOUT  タイムアウト．
*/
int  ssl_recv_mstream_Buffer(int sock, SSL* ssl, Buffer* mesg, mstream* sb, int tm)
{
    int    cc;
    unsigned char* pp;

    if (ssl==NULL) return JBXL_ARGS_ERROR;
    if (mesg==NULL || mesg->buf==NULL || sb==NULL) return JBXL_ARGS_ERROR;
    *mesg = make_Buffer(LBUF);

    if (sb->buf==NULL) {
        *sb = make_mstream(RECVBUFSZ);
        if (sb->buf==NULL) return JBXL_MALLOC_ERROR;
    }

    while (sb->datano==0) {
        cc = ssl_recv_Buffer_wait(sock, ssl, mesg, tm);
        if (cc<=0) {
            free_Buffer(mesg);
            return cc;
        }
        put_mstream(sb, mesg->buf);
        clear_Buffer(mesg);
    }

    pp = get_mstream(sb);
    if (pp==NULL) return JBXL_NET_BUF_ERROR;
    copy_s2Buffer((char*)pp, mesg);
    free(pp);

    return mesg->vldsz;
}



/**
int  ssl_recv_lines_Buffer(int sock, SSL* ssl, Buffer* mesg, int tm)

SSL経由でメッセージを受信する．複数行分（\nで終わることが保証される）のデータを
取り出さすことのできる簡易バッファ機能付き．ここからさらに一行分のデータを取り出すには，get_line() などを使用する．

ネットワークより直接一行づつ取り出すには，tcp_recv_mstream_Buffer() を使うほうが良い．

@param  sock    ソケット記述子
@param  ssl     SSL用ストリーム
@param  *mesg   受信用データバッファ．予めメモリを確保しておく．
@param  tm      タイムアウト時間．秒単位．

@retval 1以上  受信されたバイト数．
@retval 0      おそらくは相手側がセッションをクローズした．
@retval JBXL_ARGS_ERROR        引数に NULLのデータがある．
@retval JBXL_SSL_RECV_ERROR    受信エラー．
@retval JBXL_ARGS_ERROR        buf がNULL
@retval JBXL_NET_RECV_TIMEOUT  タイムアウト．

@bug 受信データが必ずLFで終わることが保障されている場合にしか使用できない．つまり汎用的には「使えない」
@see tcp_recv_mstream_Buffer() 
*/
int  ssl_recv_lines_Buffer(int sock, SSL* ssl, Buffer* mesg, int tm)
{
    int    cc;
    Buffer msg, buf;

    if (ssl==NULL || mesg==NULL || mesg->buf==NULL) return JBXL_ARGS_ERROR;
    buf = make_Buffer(LBUF);

    cc = ssl_recv_Buffer_wait(sock, ssl, &buf, tm);
    while (cc>0) {
        cat_Buffer(&buf, &msg);
        if (buf.buf[cc-1]==CHAR_LF) break;
        clear_Buffer(&buf);
        cc = ssl_recv_Buffer_wait(sock, ssl, &buf, tm);
    }
    free_Buffer(&buf);
        
    if (cc<=0) {
        free_Buffer(mesg);
        return cc;
    }

    return mesg->vldsz;
}




/////////////////////////////////////////////////////////////////
// SSL / TCP compati communications

/**    
int  ssl_tcp_recv(int sock, SSL* ssl, char* rmsg, int size)
     
sslが NULLでなければSSLで受信を行い，NULLならば通常のソケットで受信を行う．
     
@param  sock    ソケット記述子
@param  ssl     SSL用ストリーム
@param  rmsg    受信用データバッファ
@param  size    データバッファのサイズ
     
@retval 1以上  受信したバイト数．
@retval 0      おそらくは相手側がセッションをクローズした．
@retval JBXL_ARGS_ERROR        引数に NULLのデータがある．
@retval JBXL_SSL_RECV_ERROR    受信エラー．
@retval JBXL_NET_RECV_TIMEOUT  タイムアウト．

@see tcp_recv() 
*/  
int  ssl_tcp_recv(int sock, SSL* ssl, char* rmsg, int size)
{
    int cc;

    if (rmsg==NULL) return JBXL_ARGS_ERROR;
    memset(rmsg, 0, size);
    
    if (ssl!=NULL) cc = SSL_read(ssl, rmsg, size-1);
    else           cc = recv(sock, rmsg, size-1, 0);
     
    if (cc<0) cc = JBXL_SSL_RECV_ERROR;
    return cc;
}



/**    
int  ssl_tcp_send(int sock, SSL* ssl, char* smsg, int size)
     
sslが NULLでなければSSLで送信を行い，NULLならば通常のソケットで送信を行う．

データ(smsg)のサイズ sizeに0以下を指定した場合は、smsgは文字列であると見なして,サイズを自動的に計算する．
     
@param  sock   ソケット記述子
@param  ssl    SSL用ストリーム
@param  smsg   送信するデータ
@param  size   送信するデータ（smsg）のサイズ．サイズが 0以下の場合は smsgは文字列であるとみなす．
     
@retval 0以上 送信バイト数．
@retval JBXL_ARGS_ERROR      引数に NULLのデータがある．
@retval JBXL_SSL_SEND_ERROR  失敗した．

@see tcp_send() 
*/
int  ssl_tcp_send(int sock, SSL* ssl, char* smsg, int size)
{
    int cc;

    if (smsg==NULL) return JBXL_ARGS_ERROR;

    if (size<=0) size = strlen(smsg);
    if (ssl!=NULL) cc = SSL_write(ssl, smsg, size);
    else           cc = send(sock, smsg, size, 0);
    
    if (cc<0) cc = JBXL_SSL_SEND_ERROR;
    return cc;
}



/**    
int  ssl_tcp_recv_wait(int sock, SSL* ssl, char* mesg, int sz, int tm)
      
sslが NULLでなければ，SSL経由でデータを受信する．sslがNULLなら通常のソケットで受信する．

バイナリ受信も可．タイムアウトの設定が可能．タイムアウトに 0を指定した場合, 
recv_wait()関数を呼び出した時点で読み込み可能データがなければすぐにタイムアウト
となる (JBXL_NET_RECV_TIMEOUTが返る)．
   
@param  sock    ソケット記述子
@param  ssl     SSL用ストリーム
@param  mesg    受信用データバッファ
@param  sz      データバッファのサイズ
@param  tm      タイムアウト時間．秒単位．
   
@retval 1以上  受信したバイト数．
@retval 0      おそらくは相手側がセッションをクローズした．
@retval JBXL_ARGS_ERROR        引数に NULLのデータがある．
@retval JBXL_SSL_RECV_ERROR    受信エラー．
@retval JBXL_NET_RECV_TIMEOUT  タイムアウト．

@see tcp_recv_wait() 
*/  
int  ssl_tcp_recv_wait(int sock, SSL* ssl, char* mesg, int sz, int tm)
{
    int  cc;
 
    if (mesg==NULL) return JBXL_ARGS_ERROR;

    if (recv_wait(sock, tm)) {
        cc = ssl_tcp_recv(sock, ssl, mesg, sz);
        //if (cc<=0) DEBUG_MODE PRINT_MESG("SSL_TCP_RECV_WAIT: Session Closed. ret = %d. (%ds)\n", cc, tm);
    }
    else {
        //DEBUG_MODE PRINT_MESG("SSL_TCP_RECV_WAIT: Time Out. (%ds)\n", tm);
        return JBXL_NET_RECV_TIMEOUT;
    }

    return cc;
}



/**    
int  ssl_tcp_send_mesgln(int sock, SSL* ssl, char* mesg)
      
SSL or TCPメッセージ(文字列)に改行(@\r@\n)を付け加えて送信する．
   
@param  sock   ソケット記述子
@param  ssl    SSL用ストリーム
@param  mesg   送信用メッセージ
   
@retval 0以上 送信バイト数（改行を含む）．
@retval JBXL_ARGS_ERROR      引数に NULLのデータがある．
@retval JBXL_SSL_SEND_ERROR  送信失敗．
@retval JBXL_MALLOC_ERROR    メモリエラー．
*/    
int  ssl_tcp_send_mesgln(int sock, SSL* ssl, char* mesg)
{  
    int   cc, sz;
    char* buf;
    
    if (mesg==NULL) return JBXL_ARGS_ERROR;

    sz = strlen(mesg) + 3;    // for CR+LF+0x00 
    buf = (char*)malloc(sz);
    if (buf==NULL) return JBXL_MALLOC_ERROR;
    
    strncpy(buf, mesg, sz);
    strncat(buf, "\r\n", 2);
    cc = ssl_tcp_send(sock, ssl, buf, strlen(buf));
    
    free(buf);
    return cc;
}



/**
int  ssl_tcp_recv_mstream(int sock, SSL* ssl, char* mesg, int sz, mstream* sb, int tm)

SSL/TSL/TCP経由でメッセージを受信する．
 
受信メッセージはメッセージストリーム バッファに一旦バッファリングされ，この関数により一行ずつ読み出される．
mesgには最大 sz-1文字が格納される．もし，バッファ中の一行のデータが sz-1より大きい場合は，
はみ出した部分は捨てられる．

mesgに格納される時，行中の改行コードは削除され，行末には必ず '@\0' が入る．
タイムアウトの設定が可能でタイムアウトに 0を指定した場合, 呼び出した時点で
読み込み可能データがなければすぐにタイムアウトとなる (JBXL_NET_RECV_TIMEOUT が返る)．
メッセージストリームのバッファ部が確保されていない場合は，最初に呼び出された時点で確保される．

一旦この関数を使用して，受信データをバッファリングしたら，ソケットをクローズするまで，
読み取りには必ず同じストリームを使用してこの関数を呼び出さばければならない．
そうで無い場合は受信データの整合性は保証されない．

@param  sock   ソケット記述子
@param  ssl    SSL用ストリーム
@param  mesg   受信用データバッファ．予め十分なメモリ領域を確保しておく．
@param  sz     データバッファのサイズ
@param  sb     リングバッファ型のストリームバッファ．バッファ部が確保さえていなければ，自動的に確保される．
@param  tm     タイムアウト時間．秒単位．

@retval 1以上 mesgに格納したメッセージのバイト数．
@retval 0     おそらくは相手側がセッションをクローズした．
@retval JBXL_SSL_RECV_ERROR    受信エラー．
@retval JBXL_ARGS_ERROR        引数に NULLのデータがある．
@retval JBXL_MALLOC_ERROR      メッセージバッファ部が無いので確保しようとしたが，確保に失敗した．
@retval JBXL_NET_BUF_ERROR     メッセージバッファにデータは存在するはずだが，原因不明の理由により獲得に失敗した．
@retval JBXL_NET_BUFSZ_ERROR   メッセージバッファ中のデータの長さが，mesgの長さより大きい．はみ出した部分は捨てられた．
@retval JBXL_NET_RECV_TIMEOUT  タイムアウト．
*/
int  ssl_tcp_recv_mstream(int sock, SSL* ssl, char* mesg, int sz, mstream* sb, int tm)
{
    int  cc;
    unsigned char* pp;

    if (mesg==NULL || sb==NULL) return JBXL_ARGS_ERROR;
    //memset(mesg, 0, sz);

    if (sb->buf==NULL) {
        *sb = make_mstream(RECVBUFSZ);
        if (sb->buf==NULL) return JBXL_MALLOC_ERROR;
    }

    while (sb->datano==0) {
        cc = ssl_tcp_recv_wait(sock, ssl, mesg, sz, tm);
        if (cc<=0) return cc;
        put_mstream(sb, (unsigned char*)mesg);
        //memset(mesg, 0, sz);
    }

    pp = get_mstream(sb);
    if (pp==NULL) return JBXL_NET_BUF_ERROR;
    if (strlen((const char*)pp)>=(unsigned int)sz) {
        memcpy(mesg, pp, sz-1);
        free(pp);
        return JBXL_NET_BUFSZ_ERROR;
    }
    memcpy(mesg, pp, strlen((const char*)pp));

    free(pp);
    return strlen(mesg);
}



/**
int  ssl_tcp_recv_Buffer(int sock, SSL* ssl, Buffer* str)

SSL or TCP経由でデータを受信する．バッファリングなし．

@param  sock   ソケット記述子
@param  ssl    SSL用ストリーム
@param  str    受信用データバッファ．予めメモリを確保しておく．

@retval 1以上  受信バイト数．
@retval 0      正常切断
@retval JBXL_ARGS_ERROR      引数に NULLのデータがある．
@retval JBXL_SSL_RECV_ERROR  受信失敗．
*/
int  ssl_tcp_recv_Buffer(int sock, SSL* ssl, Buffer* str)
{
    int cc;

    if (str==NULL || str->buf==NULL) return JBXL_ARGS_ERROR;

    memset(str->buf, 0, str->bufsz);
    str->vldsz = 0;
    str->state = JBXL_NORMAL;

    if (ssl!=NULL) cc = SSL_read(ssl, str->buf, str->bufsz-1);
    else           cc = recv(sock, str->buf, str->bufsz-1, 0);

    if (cc>=0) str->vldsz = cc;
    else  cc = JBXL_SSL_RECV_ERROR;
    return cc;
}



/**
int  ssl_tcp_send_Buffer(int sock, SSL* ssl, Buffer* str)

SSL or TCP経由でデータを送信する．

@param  sock   ソケット記述子
@param  ssl    SSL用ストリーム
@param  str    送信用データバッファ．

@retval 0以上 送信バイト数．
@retval JBXL_ARGS_ERROR      引数に NULLのデータがある．
@retval JBXL_SSL_SEND_ERROR  送信失敗．
*/
int  ssl_tcp_send_Buffer(int sock, SSL* ssl, Buffer* str)
{
    int cc;

    if (str==NULL || str->buf==NULL) return JBXL_ARGS_ERROR;

    if (str->vldsz<0) str->vldsz = strlen((const char*)str->buf);
    if (ssl!=NULL) {
        cc = SSL_write(ssl, str->buf, str->vldsz);
    }
    else {
        cc = send(sock, str->buf, str->vldsz, 0);
    }

    if (cc<0) cc = JBXL_SSL_SEND_ERROR;
    return cc;
}



/**
int  ssl_tcp_recv_Buffer_wait(int sock, SSL* ssl, Buffer* str, int tm)

SSL or TCP経由でデータを受信する．

タイムアウトの設定が可能．タイムアウトに 0を指定した場合, recv_wait()
関数を呼び出した時点で読み込み可能データがなければすぐにタイムアウト
となる (JBXL_NET_RECV_TIMEOUTが返る)．

@param  sock   ソケット記述子
@param  ssl    SSL用ストリーム
@param  str    受信用データバッファ．予めメモリを確保しておく．
@param  tm     タイムアウト時間．秒単位．

@retval 1以上 受信したバイト数．
@retval 0       おそらくは相手側がセッションをクローズした．
@retval JBXL_ARGS_ERROR       引数に NULLのデータがある．
@retval JBXL_SSL_RECV_ERROR   受信エラー．
@retval JBXL_NET_RECV_TIMEOUT タイムアウト．
*/
int  ssl_tcp_recv_Buffer_wait(int sock, SSL* ssl, Buffer* str, int tm)
{
    int cc;

    if (str==NULL || str->buf==NULL) return JBXL_ARGS_ERROR;

    str->vldsz = 0;
    cc = ssl_tcp_recv_wait(sock, ssl, (char*)str->buf, str->bufsz, tm);
    if (cc>=0) str->vldsz = cc;

    //print_message("ssl_tcp_recv_Buffer_wait : %s\n", (char*)str->buf);
    return cc;
}



/**
int  ssl_tcp_recv_Buffer_tosize(int sock, SSL* ssl, Buffer* str, Buffer* mod, int size)

SSL or TCP経由でデータを 現データと合わせて size バイトまで受信する．

@param  sock   ソケット記述子
@param  ssl    SSL用ストリーム
@param  str    受信用データバッファ．予めメモリを確保しておく．
@param  mod    size 以上受信した場合はここに保存する．予めメモリを確保しておく．NULL でも OK
@param  size   目標バイト数．

@retval 1以上  受信バイト数．
@retval 0      正常切断
@retval JBXL_ARGS_ERROR      引数に NULLのデータがある．
@retval JBXL_SSL_RECV_ERROR  受信失敗．
*/
int  ssl_tcp_recv_Buffer_tosize(int sock, SSL* ssl, Buffer* str, Buffer* mod, int size)
{
    int cc, sz;
    int modon = FALSE;

    if (str==NULL || str->buf==NULL) return JBXL_ARGS_ERROR;
    if (mod!=NULL && mod->buf!=NULL) modon = TRUE;

    if (modon) memset(mod->buf, 0, mod->vldsz);
    Buffer buf = make_Buffer(RECVBUFSZ);

    sz = str->vldsz;
    while (sz<size) {
        cc = ssl_tcp_recv_Buffer(sock, ssl, &buf);
        if (cc>0) {
            cat_Buffer(&buf, str);
            sz += cc;
        }
        else {
            if (cc<0) sz = cc;
            break;
        }
    }
    free_Buffer(&buf);

    if (sz>size && modon) {
        copy_b2Buffer((void*)(str->buf+size), mod, sz-size);
        str->vldsz = size;
    }
    return sz;
}



/**
int  ssl_tcp_recv_Buffer_tosize_wait(int sock, SSL* ssl, Buffer* str, Buffer* mod, int size, int tm)

SSL or TCP経由でデータを 現データと合わせて size バイトまで受信する．

タイムアウトの設定が可能．タイムアウトに 0を指定した場合, recv_wait()
関数を呼び出した時点で読み込み可能データがなければすぐにタイムアウト
となる (JBXL_NET_RECV_TIMEOUTが返る)．

@param  sock   ソケット記述子
@param  ssl    SSL用ストリーム
@param  str    受信用データバッファ．予めメモリを確保しておく．
@param  mod    size 以上受信した場合はここに保存する．予めメモリを確保しておく．NULL でも OK
@param  size   目標バイト数．
@param  tm     タイムアウト時間．秒単位．

@retval 1以上 受信したバイト数．
@retval 0       おそらくは相手側がセッションをクローズした．
@retval JBXL_ARGS_ERROR       引数に NULLのデータがある．
@retval JBXL_SSL_RECV_ERROR   受信エラー．
@retval JBXL_NET_RECV_TIMEOUT タイムアウト．
*/
int  ssl_tcp_recv_Buffer_tosize_wait(int sock, SSL* ssl, Buffer* str, Buffer* mod, int size, int tm)
{
    int cc, sz;
    int modon = FALSE;

    if (str==NULL || str->buf==NULL) return JBXL_ARGS_ERROR;
    if (mod!=NULL && mod->buf!=NULL) modon = TRUE;

    if (modon) memset(mod->buf, 0, mod->vldsz);
    Buffer buf = make_Buffer(RECVBUFSZ);

    sz = str->vldsz;
    while (sz<size) {
        cc = ssl_tcp_recv_Buffer_wait(sock, ssl, &buf, tm);
        if (cc>0) {
            cat_Buffer(&buf, str);
            sz += cc;
        }
        else {
            if (cc<0) sz = cc;
            break;
        }
    }
    free_Buffer(&buf);

    if (sz>size && modon) {
        copy_b2Buffer((void*)(str->buf+size), mod, sz-size);
        str->vldsz = size;
    }
    return sz;
}



/**
int  ssl_tcp_send_sBuffer(int sock, SSL* ssl, Buffer* str)

SSL or TCP経由で文字列データを送信する．

@param  sock   ソケット記述子
@param  ssl    SSL用ストリーム
@param  str    送信用データバッファ．

@retval 0以上 送信バイト数．
@retval JBXL_ARGS_ERROR      引数に NULLのデータがある．
@retval JBXL_SSL_SEND_ERROR  送信失敗．
*/
int  ssl_tcp_send_sBuffer(int sock, SSL* ssl, Buffer* str)
{
    int cc;

    if (str==NULL || str->buf==NULL) return JBXL_ARGS_ERROR;

    if (ssl!=NULL) cc = SSL_write(ssl, str->buf, strlen((const char*)str->buf));
    else           cc = send(sock, str->buf, strlen((const char*)str->buf), 0);

    if (cc<0) cc = JBXL_SSL_SEND_ERROR;
    return cc;
}



/**
int  ssl_tcp_send_sBufferln(int sock, SSL* ssl, Buffer* str)

SSL or TCPメッセージ(文字列)に改行(@\r@\n)を付け加えて送信する．

@param  sock   ソケット記述子
@param  ssl    SSL用ストリーム
@param  str    送信用メッセージバッファ．

@retval  0以上 送信バイト数(含む改行)．
@retval JBXL_ARGS_ERROR      引数に NULLのデータがある．
@retval JBXL_SSL_SEND_ERROR  送信失敗．
*/
int  ssl_tcp_send_sBufferln(int sock, SSL* ssl, Buffer* str)
{
    int    cc;
    Buffer buf;

    if (str==NULL || str->buf==NULL) return JBXL_ARGS_ERROR;

    buf = dup_Buffer(*str);
    cat_s2Buffer("\r\n", &buf);

    if (ssl!=NULL) cc = SSL_write(ssl, buf.buf, strlen((const char*)buf.buf));
    else           cc = send(sock, buf.buf, strlen((const char*)buf.buf), 0);

    free_Buffer(&buf);

    if (cc<0) cc = JBXL_SSL_SEND_ERROR;
    return cc;
}



/**
int  ssl_tcp_recv_mstream_Buffer(int sock, SSL* ssl, Buffer* mesg, mstream* sb, int tm)

SSL or TCP経由でメッセージを受信する．受信メッセージはメッセージストリーム
に一旦バッファリングされ，この関数により一行ずつ読み出される．

mesgに格納される時，行中の改行コードは削除され，行末には必ず '@\0' が入る．
タイムアウトの設定が可能でタイムアウトに 0を指定した場合, 呼び出した時点で
読み込み可能データがなければすぐにタイムアウトとなる (JBXL_NET_RECV_TIMEOUT が返る)．
メッセージストリームのバッファ部が確保されていない場合は，最初に呼び出された時点で確保される．

一旦この関数を使用して，受信データをバッファリングしたら，ソケットを
クローズするまで，読み取りには必ず同じストリームを使用してこの関数を呼び出さばけ
ればならない．そうで無い場合は受信データの整合性は保証されない．

@param  sock    ソケット記述子
@param  ssl     SSL用ストリーム
@param  mesg    受信用データバッファ．バッファ部を確保する必要はない．
@param  sb      メッセージバッファ（リング型のストリームバッファ）．バッファ部が確保さえていなければ，自動的に確保される．
@param  tm      タイムアウト時間．秒単位．

@retval 1以上  mesgに格納したメッセージのバイト数．
@retval 0      おそらくは相手側がセッションをクローズした．
@retval JBXL_SSL_RECV_ERROR    受信エラー．
@retval JBXL_ARGS_ERROR        引数に NULLのデータがある．
@retval JBXL_MALLOC_ERROR      メッセージバッファ部が無いので確保しようとしたが，確保に失敗した．
@retval JBXL_NET_BUF_ERROR     メッセージバッファにデータは存在するはずだが，原因不明の理由により獲得に失敗した．
@retval JBXL_NET_RECV_TIMEOUT  タイムアウト．
*/
int  ssl_tcp_recv_mstream_Buffer(int sock, SSL* ssl, Buffer* mesg, mstream* sb, int tm)
{
    int    cc;
    unsigned char* pp;

    if (mesg==NULL || mesg->buf==NULL || sb==NULL) return JBXL_ARGS_ERROR;
    *mesg = make_Buffer(LBUF);

    if (sb->buf==NULL) {
        *sb = make_mstream(RECVBUFSZ);
        if (sb->buf==NULL) return JBXL_MALLOC_ERROR;
    }

    while (sb->datano==0) {
        cc = ssl_tcp_recv_Buffer_wait(sock, ssl, mesg, tm);
        if (cc<=0) {
            free_Buffer(mesg);
            return cc;
        }
        put_mstream(sb, mesg->buf);
        clear_Buffer(mesg);
    }

    pp = get_mstream(sb);
    if (pp==NULL) return JBXL_NET_BUF_ERROR;
    copy_s2Buffer((char*)pp, mesg);
    free(pp);

    return mesg->vldsz;
}



/**
int  ssl_tcp_recv_lines_Buffer(int sock, SSL* ssl, Buffer* mesg, int tm)

SSL or TCP経由でメッセージを受信する．複数行分（\nで終わることが保証される）のデータを
取り出さすことのできる簡易バッファ機能付き．ここからさらに一行分のデータを取り出すには，
get_line() などを使用する．@n
また，ネットワークより直接一行づつ取り出すには，tcp_recv_mstream_Buffer() を使うほうが良い．

@param  sock    ソケット記述子
@param  ssl     SSL用ストリーム
@param  *mesg   受信用データバッファ．予めメモリを確保しておく．
@param  tm      タイムアウト時間．秒単位．

@retval 1以上  受信されたバイト数．
@retval 0      おそらくは相手側がセッションをクローズした．
@retval JBXL_SSL_RECV_ERROR    受信エラー．
@retval JBXL_ARGS_ERROR        buf がNULL
@retval JBXL_NET_RECV_TIMEOUT  タイムアウト．

@bug 受信データが必ずLFで終わることが保障されている場合にしか使用できない．つまり汎用的には「使えない」
@see tcp_recv_mstream_Buffer()
*/
int  ssl_tcp_recv_lines_Buffer(int sock, SSL* ssl, Buffer* mesg, int tm)
{
    int    cc;
    Buffer msg, buf;

    if (mesg==NULL || mesg->buf==NULL) return JBXL_ARGS_ERROR;
    buf = make_Buffer(LBUF);

    cc = ssl_tcp_recv_Buffer_wait(sock, ssl, &buf, tm);
    while (cc>0) {
        cat_Buffer(&buf, &msg);
        if (buf.buf[cc-1]==CHAR_LF) break;
        clear_Buffer(&buf);
        cc = ssl_tcp_recv_Buffer_wait(sock, ssl, &buf, tm);
    }
    free_Buffer(&buf);
        
    if (cc<=0) {
        free_Buffer(mesg);
        return cc;
    }

    return mesg->vldsz;
}



#endif
