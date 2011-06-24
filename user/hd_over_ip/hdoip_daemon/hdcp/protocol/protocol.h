
void TX_Start(int sock);
void AKE_Init(char* rtx, int sock);
void AKE_Send_Cert(char* certrx, char* repeater, int sock);
void AKE_No_Stored_km(char* km_enc, int sock);
void AKE_Send_rrx(char* rrx, int sock);
void AKE_Send_H_prime(char* H, int sock);
void AKE_Send_Pairing_Info(char* ekh_km, int sock);
void LC_init(char* rn, int sock);
void LC_Send_L_prime(char* L_s, int sock);
void SKE_Send_Eks(char* Edkey_ks, char* riv, int sock);
void send_mess(char* data, int sockfd);
void receive_mess(char* data, int sockfd);
int start_server(char* port_nr);
int start_transmitter(char* ip_nr, char* port_nr);
int message_lenght(char buffer_start);
int check_id(char id, char expected_id);
void bbzero (char* to, int count);
void bbcopy (void const *source0, void *dest0, size_t length);
void add_leading_zeros(char* inout, int size);
void xor_strings(char* s1, char* s2, char*out, int lenght);
int generate_random_nr(char* random_nr);
int pseudo_random_nr(char* random_nr);
int hmac_sha256(char* input, char* sha);
void char_to_unsigned(char* input, unsigned char* unsig_char);


