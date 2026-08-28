#pragma once
#define _ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH

#include <cstdint>

namespace AI {
    enum GemType {
        GEMTYPE_NULL,
        GEMTYPE_I,
        GEMTYPE_T,
        GEMTYPE_L,
        GEMTYPE_J,
        GEMTYPE_Z,
        GEMTYPE_S,
        GEMTYPE_O,
    };
    typedef std::uint32_t RowBits;

    struct Gem {
        RowBits bitmap[4];
        int num, spin, mod;
        int geth() const { return 4; }
    };
    inline Gem& getGem( int number, int spin ) {
        extern Gem gems[8][4];
        return gems[number][spin];
    }
    inline int getGemColH( int number, int spin, int x) {
        extern int GEM_COL_H[8][4][4];
        return GEM_COL_H[number][spin][x];
    }

}
