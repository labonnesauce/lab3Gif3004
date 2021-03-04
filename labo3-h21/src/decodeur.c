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

int main(int argc, char* argv[]){
    
    // Écrivez le code de décodage et d'envoi sur la zone mémoire partagée ici!
    // N'oubliez pas que vous pouvez utiliser jpgd::decompress_jpeg_image_from_memory()
    // pour décoder une image JPEG contenue dans un buffer!
    // N'oubliez pas également que ce décodeur doit lire les fichiers ULV EN BOUCLE

    //Décoder le string de paramètres
    int opt;
    while((opt = getopt(argc,argv, "nortrrfifodeadline")) != -1){
        switch(opt){
            case 'nort':
                sched_setattr(0, sched_attr, sched_flags);
                break;
            case 'rr':
                sched_setattr(0, sched_priority, sched_flags);
                break;
            case 'fifo':
                sched_setattr(0, sched_priority, sched_flags);
                break;
            case 'deadline':
                attr.sched_runtime = 1;
                attr.sched_deadline = 1;
                attr.sched_period = 1;
                sched_setattr(0, sched_attr, sched_flags);
                break;
            default:
                break;
        }
    }
    for(; optind < argc; optind++){

        printf(argv[optind]);
    }

    unsigned char *imageData = decompress_jpeg_image_from_memory(const unsigned char *pSrc_data, int src_data_size,
    int *width, int *height, int *actual_comps, int req_comps);


    for(int i = 0; i < 24;i++){
        if(i­>=0 || i<=3){
            if(imageData.Content()[i]!=header[i]){
                return -1;
            }
        }
        if(i­>=4 || i<=7){
            uint32_t larg = 0;
            larg |= (uint32_t)imageData.Content()[i] << (i-4)*8);
            videoInfos->largeur = larg;
        }
    }

    //Envoyer sur la zone mémoire partagé


    return 0;
}
