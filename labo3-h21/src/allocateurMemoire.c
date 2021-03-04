#include "allocateurMemoire.h"
#include <malloc.h>

// Resize : 2*image (3 canaux) + 2*image (1 canal)
// Low-pass : 2*image (3 canaux)
// High-pass : 3*image (3 canaux)

#define ALLOC_N_BIG 16
#define ALLOC_N_SMALL 100
#define ALLOC_SMALL_SIZE 1024


// Prépare les buffers nécessaires pour une allocation correspondante aux tailles
// d'images passées en paramètre. Retourne 0 en cas de succès, et -1 si un
// problème (par exemple manque de mémoire) est survenu.
int prepareMemoire(size_t tailleImageEntree, size_t tailleImageSortie, struct memPool* pool){
    //code trouvé ici https://stackoverflow.com/questions/11749386/implement-own-memory-pool
    pool->m_poolPtr = (char *) malloc(tailleImageSortie + tailleImageEntree * 5);

    if(pool->m_poolPtr == NULL)
    {
         return -1;
    }
    
    pool->m_nextAvailAddr = pool->m_poolPtr;
    /** note the addressing starts from zero - thus you have already counted zero**/
    pool->m_endAddr = pool->m_poolPtr + tailleImageEntree - 1;
    return 0;
}

// Ces deux fonctions doivent pouvoir s'utiliser exactement comme malloc() et free()
// (dans la limite de la mémoire disponible, bien sûr)
void* tempsreel_malloc(size_t taille, struct memPool* pool){
    void *mem = NULL;
    if((pool->m_endAddr != NULL) && (pool->m_nextAvailAddr != NULL)){
        /** This is according to fencing problem - add 1 when you are find a difference of sequence to calculate the space within **/
        size_t availableSize = pool->m_endAddr - pool->m_nextAvailAddr + 1;

        if(taille > availableSize){
            mem = NULL;
        }else{
            mem = pool->m_nextAvailAddr;
            pool->m_nextAvailAddr += taille;
        }
    }
    return mem;
}

void tempsreel_free(void* ptr, struct memPool* pool){
    poolFreed *pFreed = pool->freed;
    //a checker
	//pool->freed = ptr;
	pool->freed->nextFree = pFreed;
}