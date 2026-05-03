#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_SALAS 15
/*********************************************************
 Inclua o código a seguir no seu programa, sem alterações.
 Dessa forma a saída automaticamente estará no formato esperado 
 pelo sistema de correção automática.
 *********************************************************/

void passa_tempo(int tid, int sala, int decimos)
{
    struct timespec zzz, agora;
    static struct timespec inicio = {0,0};
    int tstamp;

    if ((inicio.tv_sec == 0)&&(inicio.tv_nsec == 0)) {
        clock_gettime(CLOCK_REALTIME,&inicio);
    }

    zzz.tv_sec  = decimos/10;
    zzz.tv_nsec = (decimos%10) * 100L * 1000000L;

    if (sala==0) {
        nanosleep(&zzz,NULL);
        return;
    }

    clock_gettime(CLOCK_REALTIME,&agora);
    tstamp = ( 10 * agora.tv_sec  +  agora.tv_nsec / 100000000L )
            -( 10 * inicio.tv_sec + inicio.tv_nsec / 100000000L );

    printf("%3d [ %2d @%2d z%4d\n",tstamp,tid,sala,decimos);

    nanosleep(&zzz,NULL);

    clock_gettime(CLOCK_REALTIME,&agora);
    tstamp = ( 10 * agora.tv_sec  +  agora.tv_nsec / 100000000L )
            -( 10 * inicio.tv_sec + inicio.tv_nsec / 100000000L );

    printf("%3d ) %2d @%2d\n",tstamp,tid,sala);
}
/*********************** FIM DA FUNÇÃO *************************/
struct Posicao {
int sala_id;
int min_time;
};

struct Thread {
int id;
int init_wait;
int n_salas;
struct Posicao posicoes[30];
};

pthread_mutex_t mutex_geral = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv_sala[MAX_SALAS];

int esperando[MAX_SALAS] = {0}; //quantas threads esperam
int ocupando[MAX_SALAS] = {0};  //quantas threads estão dentro da sala
int liberados[MAX_SALAS] = {0}; //libera a entrada N de uma vez



void entra(int sala) {
    pthread_mutex_lock(&mutex_geral);
    esperando[sala]++;

    //se a sala está vazia, ninguém foi liberado, e há 3 ou mais esperando. ou seja, é a primeira vez que um trio entra na sala
    if (ocupando[sala] == 0 && liberados[sala] == 0 && esperando[sala] >= 3) {
        liberados[sala] = 3; // libera 3
        pthread_cond_broadcast(&cv_sala[sala]); //acorda todas as threads da porta
    }

    //enquanto não houver bilhete a thread dorme
    while (liberados[sala] == 0) {
        pthread_cond_wait(&cv_sala[sala], &mutex_geral);
    }

    liberados[sala]--;
    esperando[sala]--;
    ocupando[sala]++;
    
    pthread_mutex_unlock(&mutex_geral);
}


void sai(int sala) {
    pthread_mutex_lock(&mutex_geral);
    ocupando[sala]--;

    //se a sala ficou vazia
    if (ocupando[sala] == 0) {
        //se houver um trio aguardando, libera a entrada deles nessa sala
        if (esperando[sala] >= 3 && liberados[sala] == 0) {
            liberados[sala] = 3;
            pthread_cond_broadcast(&cv_sala[sala]);
        }
    }
    pthread_mutex_unlock(&mutex_geral);
}



void* rotina_thread(void* arg) {
    struct Thread* minha_thread = (struct Thread*) arg;

    //tempo de espera inicial
    passa_tempo(minha_thread->id, 0, minha_thread->init_wait);

    int sala_atual = -1;

    //movimento pelas salas
    for (int i = 0; i < minha_thread->n_salas; i++) {
        int prox_sala = minha_thread->posicoes[i].sala_id;
        int tempo_sala = minha_thread->posicoes[i].min_time;

        //entra na próxima sala
        entra(prox_sala);

        //depois de garantir a entrada na nova, libera a sala anterior
        if (sala_atual != -1) {
            sai(sala_atual);
        }

        //passa o tempo na sala nova
        passa_tempo(minha_thread->id, prox_sala, tempo_sala);

        sala_atual = prox_sala;
    }

    //ao concluir o trajeto, sai da última sala
    sai(sala_atual);

    return NULL;
}




int main(){
    int S,T;
    scanf("%d",&S);
    scanf("%d",&T);

    for(int i = 0; i <= S; i++){
        pthread_cond_init(&cv_sala[i], NULL);
    }

    struct Thread threads [T];
    pthread_t pids[T];
    for(int i = 0; i < T; i++){
        scanf("%d",&threads[i].id);
        scanf("%d",&threads[i].init_wait);
        scanf("%d",&threads[i].n_salas);
        for(int j = 0; j < threads[i].n_salas; j++){
            scanf("%d",&threads[i].posicoes[j].sala_id);
            scanf("%d",&threads[i].posicoes[j].min_time);
        }
    }

    //criação das threads
    for(int i = 0; i < T; i++){
        pthread_create(&pids[i], NULL, rotina_thread, (void*)&threads[i]);
    }

    //aguarda todas as threads finalizarem
    for(int i = 0; i < T; i++){
        pthread_join(pids[i], NULL);
    }

    //destroi as variáveis de condição após o uso
    for(int i = 0; i <= S; i++){
        pthread_cond_destroy(&cv_sala[i]);
    }
    
    return 0;

}