// Gestion des ressources et permissions
#include <sys/resource.h>

// Nécessaire pour pouvoir utiliser sched_setattr et le mode DEADLINE
#include <sched.h>
#include "schedsupp.h"

#include "allocateurMemoire.h"
#include "commMemoirePartagee.h"
#include "utils.h"


int main(int argc, char* argv[]){
    
    // Écrivez le code permettant de redimensionner une image (en utilisant les fonctions précodées
    // dans utils.c, celles commençant par "resize"). Votre code doit lire une image depuis une zone 
    // mémoire partagée et envoyer le résultat sur une autre zone mémoire partagée.
    // N'oubliez pas de respecter la syntaxe de la ligne de commande présentée dans l'énoncé.

    //Décoder le string de paramètres
    int opt;


    uint8_t sched = 0; //If the scheduling is to be changed

    struct sched_attr attr;

    attr.size = sizeof(struct sched_attr);

    int h = 0;
    int w = 0;
    int m = 0;


    while((opt = getopt(argc,argv, "s:d:w:h:m:")) != -1){
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

            case 'w':
                w = atoi(optarg);
                break;

            case 'h':
                h = atoi(optarg);
                break;

            case 'm':
                m = atoi(optarg);
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

    printf("1 %d\n", VideoIn.header->hauteur);

    prepareMemoire(h * w * VideoIn.header->canaux, VideoIn.tailleDonnees);

    printf("2 %d\n", VideoIn.header->hauteur);
    struct memPartageHeader VideoOutHeader;
    VideoOutHeader.hauteur = h;
    VideoOutHeader.largeur = w;
    VideoOutHeader.canaux = VideoIn.header->canaux;
    VideoOutHeader.fps = VideoIn.header->fps;
    VideoOut.header = &VideoOutHeader;

    printf("3 %d\n", VideoIn.header->hauteur);

    if (initMemoirePartageeEcrivain(argv[optind+1], &VideoOut, h * w * VideoIn.header->canaux, &VideoOutHeader) < 0) {
        printf("Erreur initMemoire\n");
        exit(EXIT_FAILURE);
    }

    printf("4 %d\n", VideoIn.header->hauteur);

    int in_height = VideoIn.header->hauteur;
    int in_width = VideoIn.header->largeur;

    ResizeGrid rg;
    if(m==0) {
        rg  = resizeNearestNeighborInit(h, w, in_height, in_width);
    }
    else {
        rg = resizeBilinearInit(h, w, in_height, in_width);
    }

    printf("5 %d\n", VideoIn.header->hauteur);
    printf("5 %d\n", in_height);
    printf("5 %d\n", h);

    if(sched != 0)
        sched_setattr(0, &attr, 0);


    while(1) {

        attenteLecteur(&VideoIn);
        pthread_mutex_lock(&VideoIn.header->mutex);
        VideoIn.header->frameReader++;

        if(m==0) {
            resizeNearestNeighbor(VideoIn.data, in_height, in_width,
                            VideoOut.data, h, w,
                            rg, VideoIn.header->canaux);
        }
        else {
            resizeBilinear(VideoIn.data, in_height, in_width,
                            VideoOut.data, h, w,
                            rg, VideoIn.header->canaux);
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