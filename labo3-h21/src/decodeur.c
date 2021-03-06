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
    uint32_t index = 0;
    int erreur = 0;
    int opt;
    char* value = NULL;
    int retour;
    
    char *filename;
    char *output_flux;

    int optc = 0;
    int affinity = 1;
    int core = -1;
    while((opt = getopt(argc,argv, "s:d:")) != -1){
        switch(opt){
            case 's':
                if(value* == 'NORT'){
                    retour = sched_setattr(0, sched_nice, 0);
                    if(retour == -1){
                        perror("Nice error");
                        return -1;
                    }
                }
                if(value* == 'RR'){
                    retour = sched_setattr(0, sched_priority, 0);
                    if(retour == -1){
                        perror("Round Robin error");
                        return -1;
                    }
                }
                if(value* == 'FIFO'){
                    retour = sched_setattr(0, sched_priority, 0);
                    if(retour == -1){
                        perror("FIFO error");
                        return -1;
                    }
                }
                if(value* == 'DEADLINE'){

                    attr.sched_runtime = 1;
                    attr.sched_deadline = 1;
                    attr.sched_period = 1;
                    attr.sched_policy = SCHED_DEADLINE;
                    retour = sched_setattr(0, &attr, 0);
                    if(retour == -1){
                        perror("Deadline error");
                        return -1;
                    }
                }else{

                }
                break;
            case 'd':
                attr.sched_runtime = 1;
                attr.sched_deadline = 1;
                attr.sched_period = 1;
                attr.sched_policy = SCHED_DEADLINE;

                value = optarg;
            default:
                break;
        }
    }
    argv[0]; //flux entrée
    argv[1]; //flux sortie

    //Envoyer sur la zone mémoire partagé


    return 0;
}
