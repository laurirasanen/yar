#pragma once

#define ARRAY_SIZE(A) (sizeof(A) / sizeof(*A))

#define RAND_DOUBLE(D)    (D * static_cast<double>(rand()) / RAND_MAX)
#define RAND_FLOAT(F)     (static_cast<float>(RAND(static_cast<double>(F))))
#define RAND_INT(I)       (rand() % I)
#define RAND_ELEMENT(ARR) (ARR[RAND_INT(ARRAY_SIZE(ARR))])

#define RAND_STR(LEN, STR)                                                   \
    {                                                                        \
        const char characters[] = {                                          \
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', \
            'o', 'n', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'  \
        };                                                                   \
        STR.resize(LEN);                                                     \
        for (int i = 0; i < LEN; i++)                                        \
        {                                                                    \
            STR[i] = RAND_ELEMENT(characters);                               \
        }                                                                    \
    }
