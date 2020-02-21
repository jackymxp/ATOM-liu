#ifndef MAIN_H
#define MAIN_H

inline struct ATOM* getIndexATOM(unsigned int index);
inline unsigned int getATOMSize(void);
inline void printProcessBar(double percent);

inline double distanceOfATOM(const struct ATOM* a, const struct ATOM* b);
#ifdef DST_AVE
#include "ave.h"
#endif


#ifdef DST_STRESS
#include "stress.h"
#endif


#endif/* MAIN_H */
