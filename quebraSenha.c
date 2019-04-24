// gcc quebraSenha.c -o quebraSenha -lcrypt

#include<stdio.h>
#include<crypt.h>
#include<stdlib.h>
#include <string.h>
#include <pthread.h>

#define NUM_THREADS 4

typedef struct {
    char* salt;
    char* password;
    int inicio;
    int fim;
} thread_args;

char dict[100000][32];

void *procuraPassword(void *args) {
    thread_args *t_args = (thread_args*) args;
    char *passwd;

    printf("inicio: %d, fim: %d\n", t_args->inicio, t_args->fim);
    for(int i = t_args->inicio; i < t_args->fim; i++) {
        passwd = crypt(dict[i], t_args->salt);
        if(strcmp(dict[i], "feriado") == 0)
            printf("dict_crypt: %s, shadow_crypt: %s", passwd, t_args->password);
        if(strcmp(passwd, t_args->password) == 0) {
            printf("Senha: %s\n", dict[i]);
            return NULL;
        }
    }
}

int main(int arc, char *argv[])
{
    // abrindo arquivos shadow e dicionario
    FILE* fp_shadow;
    FILE* fp_dict;
    fp_shadow = fopen(argv[1], "r");
    fp_dict = fopen("wordlist.txt", "r");

    if(fp_shadow == NULL){
        printf("Falha ao abrir o arquivo shadow.\n");
        return 0;
    }
    if(fp_dict == NULL){
        printf("Falha ao abrir o dicionario.\n");
        return 0;
    }

    // alocando variavel para receber o dicionario
    int i=0;

    // lendo arquivo shadow
    char user[32];
    char user_salt[12];
    char user_password[127];
    char aux;

    while(!feof(fp_shadow)){

        if((aux = fgetc(fp_shadow)) != EOF) user[0] = aux;
        else break;

        i=1;
        do {
            user[i] = fgetc(fp_shadow);
            i++;
        } while (user[i-1] != ':');
        user[i-1] = '\0';

        printf("User: %s\n", user);

        for(i=0; i<12; i++){
            user_salt[i] = fgetc(fp_shadow);
        }
        user_salt[i] = '\0';

        printf("Salt: %s\n", user_salt);

        i=0;
        do {
            user_password[i] = fgetc(fp_shadow);
            i++;
        } while(user_password[i-1] != ':');
        user_password[i-1] = '\0';

        printf("Password: %s\n", user_password);
        
        thread_args t_args[NUM_THREADS];
        pthread_t threads[NUM_THREADS];

        //lendo o dicionario
        i=0;
        while (!feof(fp_dict)) {
            fgets(dict[i], 32, fp_dict);
            if( strlen(dict[i]) > 0)
                dict[i][strlen(dict[i])-1] = '\0';

            if(strcmp("AAC", dict[i]) == 0) printf("SIM");

            if (++i == 100000) {
                for(i = 0; i < NUM_THREADS; i++) {
                    t_args[i].salt = user_salt;
                    t_args[i].password = user_password;
                    t_args[i].inicio = i*100000/NUM_THREADS;
                    t_args[i].fim = ((i+1) * 100000/NUM_THREADS)-1;
                    pthread_create(&(threads[i]), NULL, procuraPassword, &(t_args[i]));
                }
                for(i = 0; i < NUM_THREADS; i++) pthread_join(threads[i], NULL);
                i=0;
            }
        }
                
        while (fgetc(fp_shadow) != '\n');
    }

    fclose(fp_shadow);
    fclose(fp_dict);

    // char salt[] = "asda";
    // char *password;
    // password = crypt("senha", salt);

    // puts(password);
    return 0;
}