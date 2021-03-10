// Gestion des ressources et permissions
#include <sys/resource.h>

// Nécessaire pour pouvoir utiliser sched_setattr et le mode DEADLINE
#include <sched.h>
#include "schedsupp.h"

#include "allocateurMemoire.h"
#include "commMemoirePartagee.h"
#include "utils.h"

#include "jpgd.h"

// Définition de diverses structures pouvant vous être utiles pour la lecture d'un fichier ULV
#define HEADER_SIZE 4
const char header[] = "SETR";

struct videoInfos{
        uint32_t largeur;
        uint32_t hauteur;
        uint32_t canaux;
        uint32_t fps;
};
struct sched_attr attr;

/******************************************************************************
* FORMAT DU FICHIER VIDEO
* Offset     Taille     Type      Description
* 0          4          char      Header (toujours "SETR" en ASCII)
* 4          4          uint32    Largeur des images du vidéo
* 8          4          uint32    Hauteur des images du vidéo
* 12         4          uint32    Nombre de canaux dans les images
* 16         4          uint32    Nombre d'images par seconde (FPS)
* 20         4          uint32    Taille (en octets) de la première image -> N
* 24         N          char      Contenu de la première image (row-first)
* 24+N       4          uint32    Taille (en octets) de la seconde image -> N2
* 24+N+4     N2         char      Contenu de la seconde image
* 24+N+N2    4          uint32    Taille (en octets) de la troisième image -> N2
* ...                             Toutes les images composant la vidéo, à la suite
*            4          uint32    0 (indique la fin du fichier)
******************************************************************************/

//Valide l'entête du fichier video reçu
int validate_header(const char *actual_header) {
    return strncmp(header, actual_header, 4) != 0;
}

int main(int argc, char* argv[]){
    
    // Écrivez le code de décodage et d'envoi sur la zone mémoire partagée ici!
    // N'oubliez pas que vous pouvez utiliser jpgd::decompress_jpeg_image_from_memory()
    // pour décoder une image JPEG contenue dans un buffer!
    // N'oubliez pas également que ce décodeur doit lire les fichiers ULV EN BOUCLE

    //Décoder le string de paramètres
    int opt;


    uint8_t sched = 0; //If the scheduling is to be changed

    attr.size = sizeof(struct sched_attr);


    while((opt = getopt(argc,argv, "s:d:")) != -1){
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
                }else{

                }
                break;
            case 'd':
                attr.sched_runtime = (__u64)atoi(strtok(optarg,","));
                attr.sched_deadline = (__u64)atoi(strtok(optarg,",")); 
                attr.sched_period = (__u64)atoi(strtok(optarg,",")); 
                attr.sched_policy = SCHED_DEADLINE;

                break;
            default:
                break;
        }
    }
    
    int input_vid = open(argv[optind], O_RDONLY);

    if (input_vid < 0){
        printf("Fichier d'entree invalide\n");
        exit(EXIT_FAILURE);
    }

    struct stat videoSt;

    fstat(input_vid, &videoSt);

    const char *videoMem = (char*)mmap(NULL, videoSt.st_size, PROT_READ,MAP_POPULATE | MAP_PRIVATE, input_vid, 0);

    int headerVal = validate_header(videoMem);

    if (headerVal) {
        printf("Header invalide\n");
        exit(EXIT_FAILURE);
    }


    //Envoyer sur la zone mémoire partagé

    struct videoInfos VideoInf;
    memcpy(&VideoInf.largeur,videoMem+4 ,4);
    memcpy(&VideoInf.hauteur,videoMem+8 ,4);
    memcpy(&VideoInf.canaux, videoMem+12,4);
    memcpy(&VideoInf.fps, videoMem+16,4);

    if(prepareMemoire(VideoInf.largeur * VideoInf.hauteur * VideoInf.canaux, 0) < 0) {
        printf("Erreur Memoire\n");
        exit(EXIT_FAILURE);
    }

    struct memPartage memPart;
    struct memPartageHeader memPartHeader;
    memPartHeader.hauteur = VideoInf.hauteur;
    memPartHeader.largeur = VideoInf.largeur;
    memPartHeader.canaux = VideoInf.canaux;
    memPartHeader.fps = VideoInf.fps;
    memPart.header = &memPartHeader;

    if (initMemoirePartageeEcrivain(argv[optind+1], &memPart, VideoInf.largeur * VideoInf.hauteur * VideoInf.canaux, &memPartHeader) < 0) {
        printf("Erreur initMemoire\n");
        exit(EXIT_FAILURE);
    }

    if(sched != 0)
        sched_setattr(0, &attr, 0);

    uint32_t offset = 20;


    while(1) {
        uint32_t taille;
        memcpy(&taille, videoMem + offset, 4);

        if(taille == 0) {
            offset = 20;
            memcpy(&taille, videoMem + offset, 4);
        }

        offset += 4;

        int largeur = memPartHeader.largeur;
        int hauteur = memPartHeader.hauteur;
        int cannauxActual;

        unsigned char* image = jpgd::decompress_jpeg_image_from_memory((const unsigned char*)(videoMem+offset), taille, &largeur, &hauteur, &cannauxActual, memPartHeader.canaux);

        offset += taille;

        if(largeur != (int) memPart.header->largeur ||
            hauteur != (int) memPart.header->hauteur ||
            cannauxActual != (int) memPartHeader.canaux) {
                printf("Erreur decompression\n");
                printf("%d, %d, %d \n", (int) memPart.header->largeur, (int) memPart.header->hauteur, (int) memPartHeader.canaux);
                exit(EXIT_FAILURE);
        }

        memcpy(memPart.data,image,largeur*hauteur*cannauxActual);

        tempsreel_free(image);

        memPart.copieCompteur = memPart.header->frameReader;

        pthread_mutex_unlock(&memPart.header->mutex);
        
        attenteEcrivain(&memPart); 
        pthread_mutex_lock(&memPart.header->mutex);
        memPart.header->frameWriter++;

    }

    return 0;
}
