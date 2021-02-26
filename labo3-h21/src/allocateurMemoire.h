#ifndef ALLOC_MEM_H
#define ALLOC_MEM_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// Resize : 2*image (3 canaux) + 2*image (1 canal)
// Low-pass : 2*image (3 canaux)
// High-pass : 3*image (3 canaux)

#define ALLOC_N_BIG 16
#define ALLOC_N_SMALL 100
#define ALLOC_SMALL_SIZE 1024


typedef struct poolFreed{
	struct poolFreed *nextFree;
} poolFreed;

struct memPool
{
    poolFreed *freed;
    char* m_poolPtr;
    char* m_nextAvailAddr;
    char* m_endAddr;
};
// Prépare les buffers nécessaires pour une allocation correspondante aux tailles
// d'images passées en paramètre. Retourne 0 en cas de succès, et -1 si un
// problème (par exemple manque de mémoire) est survenu.
int prepareMemoire(size_t tailleImageEntree, size_t tailleImageSortie, struct memPool* pool);

// Ces deux fonctions doivent pouvoir s'utiliser exactement comme malloc() et free()
// (dans la limite de la mémoire disponible, bien sûr)
void* tempsreel_malloc(size_t taille, struct memPool* pool);

void tempsreel_free(void* ptr, struct memPool* pool);

// N'oubliez pas de créer le fichier allocateurMemoire.c et d'y implémenter les fonctions décrites ici!

#endif
