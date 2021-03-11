// Gestion des ressources et permissions
#include <sys/resource.h>

// Nécessaire pour pouvoir utiliser sched_setattr et le mode DEADLINE
#include <sched.h>
#include "schedsupp.h"

#include "allocateurMemoire.h"
#include "commMemoirePartagee.h"
#include "utils.h"


int main(int argc, char* argv[]){
    
    // Écrivez le code permettant de filtrer une image (en utilisant les fonctions précodées
    // dans utils.c). Votre code doit lire une image depuis une zone mémoire partagée et
    // envoyer le résultat sur une autre zone mémoire partagée.
    // N'oubliez pas de respecter la syntaxe de la ligne de commande présentée dans l'énoncé.
    int opt;


    uint8_t sched = 0; //If the scheduling is to be changed

    struct sched_attr attr;

    attr.size = sizeof(struct sched_attr);

    int t = 0;


    while((opt = getopt(argc,argv, "s:d:t:")) != -1){
        switch(opt){
            case 's':
                if(optarg[0] == 'R'){
                    sched = 1;
                    attr.sched_policy = SCHED_RR;
                }
                if(optarg[0] == 'F'){
                    sched = 1;
                    attr.sched_policy = SCHED_FIFO;
                }
                if(optarg[0] == 'D'){
                    sched = 1;
                    attr.sched_runtime = 1;
                    attr.sched_deadline = 1;
                    attr.sched_period = 1;
                    attr.sched_policy = SCHED_DEADLINE;
                }
                break;
            case 'd':
                attr.sched_runtime = (__u64)atoi(strtok(optarg,","));
                attr.sched_deadline = (__u64)atoi(strtok(optarg,",")); 
                attr.sched_period = (__u64)atoi(strtok(optarg,",")); 
                attr.sched_policy = SCHED_DEADLINE;

                break;

            case 't':
                t = atoi(optarg);
                break;

            default:
                break;
        }
    }

    struct memPartage VideoIn;
    struct memPartage VideoOut;


    if(initMemoirePartageeLecteur(argv[optind], &VideoIn) != 0){
        printf("Erreur init Memoire \n");
        exit(EXIT_FAILURE);
    }


    prepareMemoire(VideoIn.tailleDonnees,0);


    struct memPartageHeader VideoOutHeader;
    VideoOutHeader.hauteur = VideoIn.header->hauteur;
    VideoOutHeader.largeur = VideoIn.header->largeur;
    VideoOutHeader.canaux = VideoIn.header->canaux;
    VideoOutHeader.fps = VideoIn.header->fps;


    if (initMemoirePartageeEcrivain(argv[optind+1], &VideoOut, VideoOutHeader.hauteur * VideoOutHeader.largeur * VideoIn.header->canaux, &VideoOutHeader) < 0) {
        printf("Erreur initMemoire\n");
        exit(EXIT_FAILURE);
    }


    int in_height = VideoIn.header->hauteur;
    int in_width = VideoIn.header->largeur;


    if(sched != 0)
        sched_setattr(0, &attr, 0);



    while(1) {

        attenteLecteur(&VideoIn);
        pthread_mutex_lock(&VideoIn.header->mutex);
        VideoIn.header->frameReader++;

        if (t==0) {
            lowpassFilter(in_height, in_width, VideoIn.data, VideoOut.data,
                   3, 5, VideoIn.header->canaux);
        }
        else {
            highpassFilter(in_height, in_width, VideoIn.data, VideoOut.data,
                   3, 5, VideoIn.header->canaux);
        }

        VideoIn.copieCompteur = VideoIn.header->frameWriter;

		pthread_mutex_unlock(&VideoIn.header->mutex);

        VideoOut.copieCompteur = VideoOut.header->frameReader;

        pthread_mutex_unlock(&VideoOut.header->mutex);
        
        attenteEcrivain(&VideoOut); 
        pthread_mutex_lock(&VideoOut.header->mutex);
        VideoOut.header->frameWriter++;

    }

    return 0;
}