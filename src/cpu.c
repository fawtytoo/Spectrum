#include "types.h"

#include "cpu.h"

// registers -------------------------------------------------------------------
#define regA        regAFu->h
#define regF        regAFu->l
#define regAF       regAFu->w
#define regB        regBCu->h
#define regC        regBCu->l
#define regBC       regBCu->w
#define regD        regDEu->h
#define regE        regDEu->l
#define regDE       regDEu->w
#define regH        regHLu->h
#define regL        regHLu->l
#define regHL       regHLu->w

#define regIX       regIXu.w

#define regSPh      regSPu.h
#define regSPl      regSPu.l
#define regSP       regSPu.w
#define regW        regWZu.h
#define regZ        regWZu.l
#define regWZ       regWZu.w
#define regI        regIRu.h
#define regR        regIRu.l
#define regIR       regIRu.w
#define regPCh      regPCu.h
#define regPCl      regPCu.l
#define regPC       regPCu.w

#define regV        regVQu.h
#define regQ        regVQu.l
#define regVQ       regVQu.w

#define regHx       regHLp->h
#define regLx       regHLp->l
#define regHLx      regHLp->w

#define USE_HL      regHLp = regHLu; usingHL = 1
#define USE_IX      regHLp = &regIXu; usingHL = 0
#define USE_IY      regHLp = &regIYu; usingHL = 0

#define REG_DE      &regMain[regFF[regFF[1]]]
#define REG_HL      &regMain[regFF[regFF[1]] ^ 2]

typedef union
{
    WORD    w;
    struct
    {
        BYTE    l;
        BYTE    h;
    };
}
REG;

static REG          regMain[8];

static REG          *regAFu = &regMain[0];
static REG          *regBCu = &regMain[2];
static REG          *regDEu = &regMain[4];
static REG          *regHLu = &regMain[6];

static REG          regIXu;
static REG          regIYu;

static REG          regSPu;
static REG          regWZu;
static REG          regIRu;
static REG          regPCu;

static REG          regVQu; // (not an actual register)

static REG          *regHLp = &regMain[6]; // exchangeable with HL/IX/IY
static int          usingHL = 1;

static int          regFF[4] = {0, 2, 4, 5};

// flags -----------------------------------------------------------------------
#define SF              128
#define ZF              64
#define HF              16
#define PF              4
#define VF              PF
#define NF              2
#define CF              1

#define IS_SF(b)        ((b) & SF)
#define IS_ZF(b)        (IS_SZP(b) & ZF)
#define IS_HF(b)        ((b) & HF)
#define IS_VF(b)        (((b) >> 5) & VF)                       // VF = SF
#define IS_CF(b)        ((b) & CF)

#define SET_PF(b)       regQ |= (((IS_SZP(b) ^ ZF) >> 4) & PF)  // PF = !ZF
#define IFF2_PF         ((intFF2 << 2) & PF)

#define QFLAG(f)        (regQ & (f))
#define FFLAG(f)        (regF & (f))

#define IS_SZP(b)       lutSZP[b]

static const BYTE       lutSZP[256] =
{
    ZF|PF,     0,     0,    PF,     0,    PF,    PF,     0,     0,    PF,    PF,     0,    PF,     0,     0,    PF,
        0,    PF,    PF,     0,    PF,     0,     0,    PF,    PF,     0,     0,    PF,     0,    PF,    PF,     0,
        0,    PF,    PF,     0,    PF,     0,     0,    PF,    PF,     0,     0,    PF,     0,    PF,    PF,     0,
       PF,     0,     0,    PF,     0,    PF,    PF,     0,     0,    PF,    PF,     0,    PF,     0,     0,    PF,
        0,    PF,    PF,     0,    PF,     0,     0,    PF,    PF,     0,     0,    PF,     0,    PF,    PF,     0,
       PF,     0,     0,    PF,     0,    PF,    PF,     0,     0,    PF,    PF,     0,    PF,     0,     0,    PF,
       PF,     0,     0,    PF,     0,    PF,    PF,     0,     0,    PF,    PF,     0,    PF,     0,     0,    PF,
        0,    PF,    PF,     0,    PF,     0,     0,    PF,    PF,     0,     0,    PF,     0,    PF,    PF,     0,
       SF, SF|PF, SF|PF,    SF, SF|PF,    SF,    SF, SF|PF, SF|PF,    SF,    SF, SF|PF,    SF, SF|PF, SF|PF,    SF,
    SF|PF,    SF,    SF, SF|PF,    SF, SF|PF, SF|PF,    SF,    SF, SF|PF, SF|PF,    SF, SF|PF,    SF,    SF, SF|PF,
    SF|PF,    SF,    SF, SF|PF,    SF, SF|PF, SF|PF,    SF,    SF, SF|PF, SF|PF,    SF, SF|PF,    SF,    SF, SF|PF,
       SF, SF|PF, SF|PF,    SF, SF|PF,    SF,    SF, SF|PF, SF|PF,    SF,    SF, SF|PF,    SF, SF|PF, SF|PF,    SF,
    SF|PF,    SF,    SF, SF|PF,    SF, SF|PF, SF|PF,    SF,    SF, SF|PF, SF|PF,    SF, SF|PF,    SF,    SF, SF|PF,
       SF, SF|PF, SF|PF,    SF, SF|PF,    SF,    SF, SF|PF, SF|PF,    SF,    SF, SF|PF,    SF, SF|PF, SF|PF,    SF,
       SF, SF|PF, SF|PF,    SF, SF|PF,    SF,    SF, SF|PF, SF|PF,    SF,    SF, SF|PF,    SF, SF|PF, SF|PF,    SF,
    SF|PF,    SF,    SF, SF|PF,    SF, SF|PF, SF|PF,    SF,    SF, SF|PF, SF|PF,    SF, SF|PF,    SF,    SF, SF|PF
};

// ALU -------------------------------------------------------------------------
#define ALU_ADD         regQ = ZF; ALU_Add(regA, regV, 0); regAF = regVQ
#define ALU_ADC         regQ = ZF; ALU_Add(regA, regV, FFLAG(CF)); regAF = regVQ
#define ALU_SUB         regQ = ZF; ALU_Sub(regA, regV, 0); regAF = regVQ
#define ALU_SBC         regQ = ZF; ALU_Sub(regA, regV, FFLAG(CF)); regAF = regVQ

// ops -------------------------------------------------------------------------
#define CYCLEINC(i)     cpuOpCycleInc = i
#define OPCYCLE(c)      CYCLEINC(0); cpuOpCycle[0] = c
#define M1_FETCH(t)     cpuOpTable = t; OPCYCLE(3)
#define OP_FETCH        M1_FETCH(0); USE_HL; if (CPU_INT & intFF1) { intFF1 = intFF2 = 0; OPCYCLE(6); if (CPU_HALT) { CPU_HALT = 0; regPC++; } }
#define IRh             (cpuIR >> 3) & 7

#define NOP             25
#define OP22            148
#define OP2A            188

static int              cpuOpCycle[2] = {0, 0};
static int              cpuOpCycleInc = 0;

static int              cpuOpTable = 0;
static BYTE             cpuIR;              // instruction register

static const int        cpuOp[4][256] =
{
    {
         NOP,   26,   33,   37,   40,   41,   42,   46,   47,   48,   56,   60,   63,   64,   65,   69,
          70,   80,   87,   91,   94,   95,   96,  100,  101,  110,  118,  122,  125,  126,  127,  131,
         132,  141, OP22,  161,  164,  165,  166,  170,  171,  180, OP2A,  201,  204,  205,  206,  210,
         211,  220,  227,  237,  240,  256,  272,  284,  285,  294,  302,  312,  315,  316,  317,  321,
         322,  323,  324,  325,  326,  327,  328,  340,  341,  342,  343,  344,  345,  346,  347,  359,
         360,  361,  362,  363,  364,  365,  366,  378,  379,  380,  381,  382,  383,  384,  385,  397,
         398,  399,  400,  401,  402,  403,  404,  416,  417,  418,  419,  420,  421,  422,  423,  435,
         436,  448,  460,  472,  484,  496,  508,  509,  521,  522,  523,  524,  525,  526,  527,  539,
         540,  541,  542,  543,  544,  545,  546,  558,  559,  560,  561,  562,  563,  564,  565,  577,
         578,  579,  580,  581,  582,  583,  584,  596,  597,  598,  599,  600,  601,  602,  603,  615,
         616,  617,  618,  619,  620,  621,  622,  634,  635,  636,  637,  638,  639,  640,  641,  653,
         654,  655,  656,  657,  658,  659,  660,  672,  673,  674,  675,  676,  677,  678,  679,  691,
         692,  700,  707,  714,  721,  735,  743,  747,  755,  763,  770,  777,  789,  803,  817,  821,
         829,  837,  844,  851,  859,  873,  881,  885,  893,  901,  902,  909,  917,  931,  932,  936,
         944,  952,  959,  966,  982,  996, 1004, 1008, 1016, 1024, 1025, 1032, 1033, 1047, 1048, 1052,
        1060, 1068, 1075, 1082, 1083, 1097, 1105, 1109, 1117, 1125, 1128, 1135, 1136, 1150, 1151, 1155
    },
    {
         NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,
         NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,
         NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,
         NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,
        1163, 1168, 1173, 1181, 1194, 1195, 1202, 1203, 1205, 1210, 1215, 1223, 1194, 1236, 1202, 1243,
        1245, 1250, 1255, 1263, 1194, 1195, 1276, 1277, 1279, 1284, 1289, 1297, 1194, 1195, 1310, 1311,
        1313, 1318, 1323, OP22, 1194, 1195, 1202, 1331, 1342, 1347, 1352, OP2A, 1194, 1195, 1202, 1360,
        1371, 1376, 1381, 1389, 1194, 1195, 1276,  NOP, 1402, 1407, 1412, 1420, 1194, 1195, 1310,  NOP,
         NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,
         NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,
        1433, 1442, 1451, 1460,  NOP,  NOP,  NOP,  NOP, 1469, 1478, 1487, 1496,  NOP,  NOP,  NOP,  NOP,
        1505, 1519, 1533, 1547,  NOP,  NOP,  NOP,  NOP, 1561, 1575, 1589, 1603,  NOP,  NOP,  NOP,  NOP,
         NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,
         NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,
         NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,
         NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP,  NOP
    },
    {
        1617, 1618, 1619, 1620, 1621, 1622, 1623, 1631, 1632, 1633, 1634, 1635, 1636, 1637, 1638, 1646,
        1647, 1648, 1649, 1650, 1651, 1652, 1653, 1661, 1662, 1663, 1664, 1665, 1666, 1667, 1668, 1676,
        1677, 1678, 1679, 1680, 1681, 1682, 1683, 1691, 1692, 1693, 1694, 1695, 1696, 1697, 1698, 1706,
        1707, 1708, 1709, 1710, 1711, 1712, 1713, 1721, 1722, 1723, 1724, 1725, 1726, 1727, 1728, 1736,
        1737, 1738, 1739, 1740, 1741, 1742, 1743, 1748, 1737, 1738, 1739, 1740, 1741, 1742, 1743, 1748,
        1737, 1738, 1739, 1740, 1741, 1742, 1743, 1748, 1737, 1738, 1739, 1740, 1741, 1742, 1743, 1748,
        1737, 1738, 1739, 1740, 1741, 1742, 1743, 1748, 1737, 1738, 1739, 1740, 1741, 1742, 1743, 1748,
        1737, 1738, 1739, 1740, 1741, 1742, 1743, 1748, 1737, 1738, 1739, 1740, 1741, 1742, 1743, 1748,
        1749, 1750, 1751, 1752, 1753, 1754, 1755, 1763, 1749, 1750, 1751, 1752, 1753, 1754, 1755, 1763,
        1749, 1750, 1751, 1752, 1753, 1754, 1755, 1763, 1749, 1750, 1751, 1752, 1753, 1754, 1755, 1763,
        1749, 1750, 1751, 1752, 1753, 1754, 1755, 1763, 1749, 1750, 1751, 1752, 1753, 1754, 1755, 1763,
        1749, 1750, 1751, 1752, 1753, 1754, 1755, 1763, 1749, 1750, 1751, 1752, 1753, 1754, 1755, 1763,
        1764, 1765, 1766, 1767, 1768, 1769, 1770, 1778, 1764, 1765, 1766, 1767, 1768, 1769, 1770, 1778,
        1764, 1765, 1766, 1767, 1768, 1769, 1770, 1778, 1764, 1765, 1766, 1767, 1768, 1769, 1770, 1778,
        1764, 1765, 1766, 1767, 1768, 1769, 1770, 1778, 1764, 1765, 1766, 1767, 1768, 1769, 1770, 1778,
        1764, 1765, 1766, 1767, 1768, 1769, 1770, 1778, 1764, 1765, 1766, 1767, 1768, 1769, 1770, 1778
    },
    {
        1779, 1783, 1784, 1785, 1786, 1787, 1788, 1789, 1790, 1794, 1795, 1796, 1797, 1798, 1799, 1800,
        1801, 1805, 1806, 1807, 1808, 1809, 1810, 1811, 1812, 1816, 1817, 1818, 1819, 1820, 1821, 1822,
        1823, 1827, 1828, 1829, 1830, 1831, 1832, 1833, 1834, 1838, 1839, 1840, 1841, 1842, 1843, 1844,
        1845, 1849, 1850, 1851, 1852, 1853, 1854, 1855, 1856, 1860, 1861, 1862, 1863, 1864, 1865, 1866,
        1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867,
        1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867,
        1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867,
        1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867, 1867,
        1868, 1872, 1873, 1874, 1875, 1876, 1877, 1878, 1868, 1872, 1873, 1874, 1875, 1876, 1877, 1878,
        1868, 1872, 1873, 1874, 1875, 1876, 1877, 1878, 1868, 1872, 1873, 1874, 1875, 1876, 1877, 1878,
        1868, 1872, 1873, 1874, 1875, 1876, 1877, 1878, 1868, 1872, 1873, 1874, 1875, 1876, 1877, 1878,
        1868, 1872, 1873, 1874, 1875, 1876, 1877, 1878, 1868, 1872, 1873, 1874, 1875, 1876, 1877, 1878,
        1879, 1883, 1884, 1885, 1886, 1887, 1888, 1889, 1879, 1883, 1884, 1885, 1886, 1887, 1888, 1889,
        1879, 1883, 1884, 1885, 1886, 1887, 1888, 1889, 1879, 1883, 1884, 1885, 1886, 1887, 1888, 1889,
        1879, 1883, 1884, 1885, 1886, 1887, 1888, 1889, 1879, 1883, 1884, 1885, 1886, 1887, 1888, 1889,
        1879, 1883, 1884, 1885, 1886, 1887, 1888, 1889, 1879, 1883, 1884, 1885, 1886, 1887, 1888, 1889
    }
};

// misc ------------------------------------------------------------------------
#define DEMUX               CPU_A0 = cpuAddress & 1;            \
                            CPU_A1 = (cpuAddress >> 1) & 1;     \
                            CPU_A2 = (cpuAddress >> 2) & 1;     \
                            CPU_A3 = (cpuAddress >> 3) & 1;     \
                            CPU_A4 = (cpuAddress >> 4) & 1;     \
                            CPU_A5 = (cpuAddress >> 5) & 1;     \
                            CPU_A6 = (cpuAddress >> 6) & 1;     \
                            CPU_A7 = (cpuAddress >> 7) & 1;     \
                            CPU_A8 = (cpuAddress >> 8) & 1;     \
                            CPU_A9 = (cpuAddress >> 9) & 1;     \
                            CPU_A10 = (cpuAddress >> 10) & 1;   \
                            CPU_A11 = (cpuAddress >> 11) & 1;   \
                            CPU_A12 = (cpuAddress >> 12) & 1;   \
                            CPU_A13 = (cpuAddress >> 13) & 1;   \
                            CPU_A14 = (cpuAddress >> 14) & 1;   \
                            CPU_A15 = cpuAddress >> 15

#define BUS_ADD(a)          cpuAddress = a; DEMUX
#define BUS_ADD_DATA(a, d)  BUS_ADD(a); cpuData = d

#define MEM_READ(a)         mreq = 1; CPU_RD = 1; BUS_ADD(a)
#define MEM_WRITE(a, d)     mreq = 1; CPU_WR = 1; BUS_ADD_DATA(a, d)
#define IO_READ             iorq = 3; CPU_RD = 1; cpuData = 0xff
#define IO_WRITE            iorq = 3; CPU_WR = 1

#define MEM_REFRESH         mreq = 1; CPU_RFSH = 1; BUS_ADD(regIR); regR = (regR & 128) | ((regR + 1) & 127)

#define DISPLACE(d)         ((d & 127) - (d & 128))
#define BIT(b)              (1 << (b))

static int                  intFF1 = 0, intFF2 = 0;
static int                  intMode = 0;

// external --------------------------------------------------------------------
short           cpuPin[40];
WORD            cpuAddress;
BYTE            cpuData;

// ALU -------------------------------------------------------------------------
static void ALU_Add(BYTE a, BYTE b, BYTE c)
{
    BYTE    d[2]; // internal 4-bit latches

    d[0] = (a & 0x0f) + (b & 0x0f) + c;
    d[1] = (a >> 4) + (b >> 4) + (d[0] >> 4);
    regV = (d[1] << 4) | (d[0] & 0x0f);

    regQ = IS_SF(regV) | QFLAG(IS_ZF(regV)) | IS_HF(d[0]) | IS_VF((a ^ regV) & (b ^ regV)) | QFLAG(NF) | IS_CF(d[1] >> 4);
}

static void ALU_Sub(BYTE a, BYTE b, BYTE c)
{
    BYTE    d[2];

    d[0] = (a & 0x0f) - (b & 0x0f) - c;
    d[1] = (a >> 4) - (b >> 4) - ((d[0] >> 4) & 1);
    regV = (d[1] << 4) | (d[0] & 0x0f);

    regQ = IS_SF(regV) | QFLAG(IS_ZF(regV)) | IS_HF(d[0]) | IS_VF((a ^ regV) & (~(b ^ regV))) | NF | IS_CF(d[1] >> 4);
}

static void ALU_And()
{
    regV &= regA;
    regQ = IS_SZP(regV) | HF;
    regAF = regVQ;
}

static void ALU_Xor()
{
    regV ^= regA;
    regQ = IS_SZP(regV);
    regAF = regVQ;
}

static void ALU_Or()
{
    regV |= regA;
    regQ = IS_SZP(regV);
    regAF = regVQ;
}

static void ALU_Cp()
{
    regQ = ZF;
    ALU_Sub(regA, regV, 0);
    regF = regQ;
}

static void ALU_Inc()
{
    regQ = ZF;
    ALU_Add(regV, 1, 0);
    regF = QFLAG(~CF) | FFLAG(CF);
}

static void ALU_Dec()
{
    regQ = ZF;
    ALU_Sub(regV, 1, 0);
    regF = QFLAG(~CF) | FFLAG(CF);
}

static void ALU_RLC()
{
    regQ = regV >> 7;
    regV <<= 1;
    regV |= regQ;
    regQ |= IS_SZP(regV);
}

static void ALU_RRC()
{
    regQ = regV & 1;
    regV >>= 1;
    regV |= (regQ << 7);
    regQ |= IS_SZP(regV);
}

static void ALU_RL()
{
    regQ = regV >> 7;
    regV <<= 1;
    regV |= FFLAG(CF);
    regQ |= IS_SZP(regV);
}

static void ALU_RR()
{
    regQ = regV & 1;
    regV >>= 1;
    regV |= (FFLAG(CF) << 7);
    regQ |= IS_SZP(regV);
}

static void ALU_SLA()
{
    regQ = regV >> 7;
    regV <<= 1;
    regQ |= IS_SZP(regV);
}

static void ALU_SRA()
{
    regQ = regV & 1;
    regV = (regV >> 1) | (regV & 128);
    regQ |= IS_SZP(regV);
}

static void ALU_SLL()
{
    regQ = regV >> 7;
    regV <<= 1;
    regV |= 1;
    regQ |= IS_SZP(regV);
}

static void ALU_SRL()
{
    regQ = regV & 1;
    regV >>= 1;
    regQ |= IS_SZP(regV);
}

static void ALU_Bit()
{
    regQ = HF | FFLAG(CF);
    regV &= BIT(IRh);
    regQ |= (IS_SZP(regV) & (~PF));
    regQ |= (QFLAG(ZF) >> 4);       // PF = ZF
    regF = regQ;
}

static void ALU_DAA()
{
    regV = regA;
    regQ = FFLAG(NF);

    if (FFLAG(NF))
    {
        if (regA > 0x99 || FFLAG(CF))
        {
            regV -= 0x60;
            regQ |= CF;
        }

        if ((regA & 0x0f) > 0x09 || FFLAG(HF))
        {
            regV -= 0x06;
        }
    }
    else
    {
        if (regA > 0x99 || FFLAG(CF))
        {
            regV += 0x60;
            regQ |= CF;
        }

        if ((regA & 0x0f) > 0x09 || FFLAG(HF))
        {
            regV += 0x06;
        }
    }

    regQ |= (IS_SZP(regV) | ((regV ^ regA) & HF));
    regAF = regVQ;
}

// ops -------------------------------------------------------------------------
void CPU_Cycle()
{
    static int      iorq = 0;
    static int      mreq = 0;

    if (iorq > 0)
    {
        iorq--;
        if (iorq == 0)
        {
            BUS_ADD(0x0000);
        }
    }
    if (mreq)
    {
        mreq = 0;
        BUS_ADD(0x0000);
    }
    CPU_RD = 0;
    CPU_WR = 0;
    CPU_M1 = 0;
    CPU_RFSH = 0;

    cpuOpCycle[0] += cpuOpCycleInc;

    CYCLEINC(1);

    switch (cpuOpCycle[CPU_RESET])
    {
      case 0: // RESET
        OPCYCLE(1); intFF1 = intFF2 = 0; intMode = 0; break;
      case 1:
        regAF = 0xffff; regSP = 0xffff; break;
      case 2:
        regPC = 0; regIR = 0; OP_FETCH; break;
      case 3: // M1_FETCH
        CPU_M1 = 1; MEM_READ(regPC++); break;
      case 4:
        cpuIR = cpuData; break;
      case 5:
        MEM_REFRESH; OPCYCLE(cpuOp[cpuOpTable][cpuIR]); break;
      case 6: // INTERRUPT (13 7,3,3)
        CPU_M1 = 1; break;
      case 7:
        break;
      case 8:
        CPU_M1 = 1; iorq = 1; break;
      case 9:
        regW = regI; regZ = cpuData; break;
      case 10:
        MEM_REFRESH; break;
      case 11:
        break;
      case 12:
        regSP--; break;
      case 13:
        MEM_WRITE(regSP, regPCh); break;
      case 14:
        regSP--; break;
      case 15:
        break;
      case 16:
        MEM_WRITE(regSP, regPCl); break;
      case 17:
        break;
      case 18:
        if (intMode != 2) { regPC = 0x38; OP_FETCH; } break;
      case 19: // mode 2 (19 +3,3)
        MEM_READ(regWZ); break;
      case 20:
        regWZ++; break;
      case 21:
        regPCl = cpuData; break;
      case 22:
        MEM_READ(regWZ); break;
      case 23:
        break;
      case 24:
        regPCh = cpuData; OP_FETCH; break;
      case NOP: // nop 4(4)
        OP_FETCH; break;
      case 26: // ld bc,NN 10(4,3,3)
        break;
      case 27:
        MEM_READ(regPC++); break;
      case 28:
        break;
      case 29:
        regC = cpuData; break;
      case 30:
        MEM_READ(regPC++); break;
      case 31:
        break;
      case 32:
        regB = cpuData; OP_FETCH; break;
      case 33: // ld (bc),a 7(4,3)
        break;
      case 34:
        MEM_WRITE(regBC, regA); break;
      case 35:
        break;
      case 36:
        OP_FETCH; break;
      case 37: // inc bc 6(6)
        break;
      case 38:
        regBC++; break;
      case 39:
        OP_FETCH; break;
      case 40: // inc b 4(4)
        regV = regB; ALU_Inc(); regB = regV; OP_FETCH; break;
      case 41: // dec b 4(4)
        regV = regB; ALU_Dec(); regB = regV; OP_FETCH; break;
      case 42: // ld b,N 7(4,3)
        break;
      case 43:
        MEM_READ(regPC++); break;
      case 44:
        break;
      case 45:
        regB = cpuData; OP_FETCH; break;
      case 46: // rlca 4(4)
        regV = regA; ALU_RLC(); regQ = FFLAG(SF | ZF | PF) | QFLAG(HF | NF | CF); regAF = regVQ; OP_FETCH; break;
      case 47: // ex af,af' 4(4)
        regFF[0] ^= 1; regAFu = &regMain[regFF[0]]; OP_FETCH; break;
      case 48: // add hl,bc 11(4,4,3) add ix/iy,bc 15(4,4,4,3)
        break;
      case 49:
        regQ = ZF; break;
      case 50:
        ALU_Add(regLx, regC, 0); break;
      case 51:
        regLx = regV; break;
      case 52:
        break;
      case 53:
        ALU_Add(regHx, regB, QFLAG(CF)); break;
      case 54:
        regHx = regV; regF = FFLAG(SF | ZF | VF) | QFLAG(HF | NF | CF); break;
      case 55:
        OP_FETCH; break;
      case 56: // ld a,(bc) 7(4,3)
        break;
      case 57:
        MEM_READ(regBC); break;
      case 58:
        break;
      case 59:
        regA = cpuData; OP_FETCH; break;
      case 60: // dec bc 6(6)
        break;
      case 61:
        regBC--; break;
      case 62:
        OP_FETCH; break;
      case 63: // inc c 4(4)
        regV = regC; ALU_Inc(); regC = regV; OP_FETCH; break;
      case 64: // dec c 4(4)
        regV = regC; ALU_Dec(); regC = regV; OP_FETCH; break;
      case 65: // ld c,N 7(4,3)
        break;
      case 66:
        MEM_READ(regPC++); break;
      case 67:
        break;
      case 68:
        regC = cpuData; OP_FETCH; break;
      case 69: // rrca 4(4)
        regV = regA; ALU_RRC(); regQ = FFLAG(SF | ZF | PF) | QFLAG(HF | NF | CF); regAF = regVQ; OP_FETCH; break;
      case 70: // djnz D 8(5,3) 13(5,3,5)
        break;
      case 71:
        break;
      case 72:
        MEM_READ(regPC++); regB--; break;
      case 73:
        break;
      case 74:
        if (regB == 0) { OP_FETCH; } break;
      case 75:
        regPC += DISPLACE(cpuData); break;
      case 76:
        break;
      case 77:
        break;
      case 78:
        break;
      case 79:
        OP_FETCH; break;
      case 80: // ld de,NN 10(4,3,3)
        break;
      case 81:
        MEM_READ(regPC++); break;
      case 82:
        break;
      case 83:
        regE = cpuData; break;
      case 84:
        MEM_READ(regPC++); break;
      case 85:
        break;
      case 86:
        regD = cpuData; OP_FETCH; break;
      case 87: // ld (de),a 7(4,3)
        break;
      case 88:
        MEM_WRITE(regDE, regA); break;
      case 89:
        break;
      case 90:
        OP_FETCH; break;
      case 91: // inc de 6(6)
        break;
      case 92:
        regDE++; break;
      case 93:
        OP_FETCH; break;
      case 94: // inc d 4(4)
        regV = regD; ALU_Inc(); regD = regV; OP_FETCH; break;
      case 95: // dec d 4(4)
        regV = regD; ALU_Dec(); regD = regV; OP_FETCH; break;
      case 96: // ld d,N 7(4,3)
        break;
      case 97:
        MEM_READ(regPC++); break;
      case 98:
        break;
      case 99:
        regD = cpuData; OP_FETCH; break;
      case 100: // rla 4(4)
        regV = regA; ALU_RL(); regQ = FFLAG(SF | ZF | PF) | QFLAG(HF | NF | CF); regAF = regVQ; OP_FETCH; break;
      case 101: // jr D 12(4,3,5)
        break;
      case 102:
        MEM_READ(regPC++); break;
      case 103:
        break;
      case 104:
        break;
      case 105:
        regPC += DISPLACE(cpuData); break;
      case 106:
        break;
      case 107:
        break;
      case 108:
        break;
      case 109:
        OP_FETCH; break;
      case 110: // add hl,de 11(4,4,3) add ix/iy,de 15(4,4,4,3)
        break;
      case 111:
        regQ = ZF; break;
      case 112:
        ALU_Add(regLx, regE, 0); break;
      case 113:
        regLx = regV; break;
      case 114:
        break;
      case 115:
        ALU_Add(regHx, regD, QFLAG(CF)); break;
      case 116:
        regHx = regV; regF = FFLAG(SF | ZF | VF) | QFLAG(HF | NF | CF); break;
      case 117:
        OP_FETCH; break;
      case 118: // ld a,(de) 7(4,3)
        break;
      case 119:
        MEM_READ(regDE); break;
      case 120:
        break;
      case 121:
        regA = cpuData; OP_FETCH; break;
      case 122: // dec de 6(6)
        break;
      case 123:
        regDE--; break;
      case 124:
        OP_FETCH; break;
      case 125: // inc e 4(4)
        regV = regE; ALU_Inc(); regE = regV; OP_FETCH; break;
      case 126: // dec e 4(4)
        regV = regE; ALU_Dec(); regE = regV; OP_FETCH; break;
      case 127: // ld e,N 7(4,3)
        break;
      case 128:
        MEM_READ(regPC++); break;
      case 129:
        break;
      case 130:
        regE = cpuData; OP_FETCH; break;
      case 131: // rra 4(4)
        regV = regA; ALU_RR(); regQ = FFLAG(SF | ZF | PF) | QFLAG(HF | NF | CF); regAF = regVQ; OP_FETCH; break;
      case 132: // jr nz,D 7(4,3) 12(4,3,5)
        break;
      case 133:
        MEM_READ(regPC++); break;
      case 134:
        break;
      case 135:
        if (FFLAG(ZF)) { OP_FETCH; } break;
      case 136:
        regPC += DISPLACE(cpuData); break;
      case 137:
        break;
      case 138:
        break;
      case 139:
        break;
      case 140:
        OP_FETCH; break;
      case 141: // ld hl,NN 10(4,3,3) ld ix/iy,NN 14(4,4,3,3)
        break;
      case 142:
        MEM_READ(regPC++); break;
      case 143:
        break;
      case 144:
        regLx = cpuData; break;
      case 145:
        MEM_READ(regPC++); break;
      case 146:
        break;
      case 147:
        regHx = cpuData; OP_FETCH; break;
      case OP22: // ld (NN),hl 16(4,3,3,3,3) ld (NN),ix/iy 20(4,4,3,3,3,3) NOTE: also ED 63
        break;
      case 149:
        MEM_READ(regPC++); break;
      case 150:
        break;
      case 151:
        regZ = cpuData; break;
      case 152:
        MEM_READ(regPC++); break;
      case 153:
        break;
      case 154:
        regW = cpuData; break;
      case 155:
        MEM_WRITE(regWZ, regLx); break;
      case 156:
        regWZ++; break;
      case 157:
        break;
      case 158:
        MEM_WRITE(regWZ, regHx); break;
      case 159:
        break;
      case 160:
        OP_FETCH; break;
      case 161: // inc hl 6(6) inc ix/iy 10(4,6)
        break;
      case 162:
        regHLx++; break;
      case 163:
        OP_FETCH; break;
      case 164: // inc h 4(4) inc ixh/iyh 8(4,4)
        regV = regHx; ALU_Inc(); regHx = regV; OP_FETCH; break;
      case 165: // dec h 4(4) dec ixh/iyh 8(4,4)
        regV = regHx; ALU_Dec(); regHx = regV; OP_FETCH; break;
      case 166: // ld h,N 7(4,3) ld ixh/iyh,N 11(4,4,3)
        break;
      case 167:
        MEM_READ(regPC++); break;
      case 168:
        break;
      case 169:
        regHx = cpuData; OP_FETCH; break;
      case 170: // daa 4(4)
        ALU_DAA(); OP_FETCH; break;
      case 171: // jr z,D 7(4,3) 12(4,3,5)
        break;
      case 172:
        MEM_READ(regPC++); break;
      case 173:
        break;
      case 174:
        if (!FFLAG(ZF)) { OP_FETCH; } break;
      case 175:
        regPC += DISPLACE(cpuData); break;
      case 176:
        break;
      case 177:
        break;
      case 178:
        break;
      case 179:
        OP_FETCH; break;
      case 180: // add hl,hl 11(4,4,3) add ix/iy,ix/iy 15(4,4,4,3)
        break;
      case 181:
        regQ = ZF; break;
      case 182:
        ALU_Add(regLx, regLx, 0); break;
      case 183:
        regLx = regV; break;
      case 184:
        break;
      case 185:
        ALU_Add(regHx, regHx, QFLAG(CF)); break;
      case 186:
        regHx = regV; regF = FFLAG(SF | ZF | VF) | QFLAG(HF | NF | CF); break;
      case 187:
        OP_FETCH; break;
      case OP2A: // ld hl,(NN) 16(4,3,3,3,3) ld ix/iy,(NN) 20(4,4,3,3,3,3) NOTE: also ED 6B
        break;
      case 189:
        MEM_READ(regPC++); break;
      case 190:
        break;
      case 191:
        regZ = cpuData; break;
      case 192:
        MEM_READ(regPC++); break;
      case 193:
        break;
      case 194:
        regW = cpuData; break;
      case 195:
        MEM_READ(regWZ); break;
      case 196:
        regWZ++; break;
      case 197:
        regLx = cpuData; break;
      case 198:
        MEM_READ(regWZ); break;
      case 199:
        break;
      case 200:
        regHx = cpuData; OP_FETCH; break;
      case 201: // dec hl 6(6) dec ix/iy 10(4,6)
        break;
      case 202:
        regHLx--; break;
      case 203:
        OP_FETCH; break;
      case 204: // inc l 4(4) inc ixl/iyl 8(4,4)
        regV = regLx; ALU_Inc(); regLx = regV; OP_FETCH; break;
      case 205: // dec l 4(4) dec ixl/iyl 8(4,4)
        regV = regLx; ALU_Dec(); regLx = regV; OP_FETCH; break;
      case 206: // ld l,N 7(4,3) ld ixl/iyl,N 11(4,4,3)
        break;
      case 207:
        MEM_READ(regPC++); break;
      case 208:
        break;
      case 209:
        regLx = cpuData; OP_FETCH; break;
      case 210: // cpl 4(4)
        regV = regA ^ 255; regQ = regF | (HF | NF); regAF = regVQ; OP_FETCH; break;
      case 211: // jr nc,D 7(4,3) 12(4,3,5)
        break;
      case 212:
        MEM_READ(regPC++); break;
      case 213:
        break;
      case 214:
        if (FFLAG(CF)) { OP_FETCH; } break;
      case 215:
        regPC += DISPLACE(cpuData); break;
      case 216:
        break;
      case 217:
        break;
      case 218:
        break;
      case 219:
        OP_FETCH; break;
      case 220: // ld sp,NN 10(4,3,3)
        break;
      case 221:
        MEM_READ(regPC++); break;
      case 222:
        break;
      case 223:
        regSPl = cpuData; break;
      case 224:
        MEM_READ(regPC++); break;
      case 225:
        break;
      case 226:
        regSPh = cpuData; OP_FETCH; break;
      case 227: // ld (NN),a 13(4,3,3,3)
        break;
      case 228:
        MEM_READ(regPC++); break;
      case 229:
        break;
      case 230:
        regZ = cpuData; break;
      case 231:
        MEM_READ(regPC++); break;
      case 232:
        break;
      case 233:
        regW = cpuData; break;
      case 234:
        MEM_WRITE(regWZ, regA); break;
      case 235:
        break;
      case 236:
        OP_FETCH; break;
      case 237: // inc sp 6(6)
        break;
      case 238:
        regSP++; break;
      case 239:
        OP_FETCH; break;
      case 240: // inc (hl) 11(4,4,3) inc (ix/iy+D) 23(4,4,3,5,4,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 241:
        MEM_READ(regPC++); break;
      case 242:
        break;
      case 243:
        break;
      case 244:
        regWZ += DISPLACE(cpuData); break;
      case 245:
        break;
      case 246:
        break;
      case 247:
        break;
      case 248:
        break;
      case 249:
        MEM_READ(regWZ); break;
      case 250:
        break;
      case 251:
        regV = cpuData; break;
      case 252:
        ALU_Inc(); break;
      case 253:
        MEM_WRITE(regWZ, regV); break;
      case 254:
        break;
      case 255:
        OP_FETCH; break;
      case 256: // dec (hl) 11(4,4,3) dec (ix/iy+D) 23(4,4,3,5,4,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 257:
        MEM_READ(regPC++); break;
      case 258:
        break;
      case 259:
        break;
      case 260:
        regWZ += DISPLACE(cpuData); break;
      case 261:
        break;
      case 262:
        break;
      case 263:
        break;
      case 264:
        break;
      case 265:
        MEM_READ(regWZ); break;
      case 266:
        break;
      case 267:
        regV = cpuData; break;
      case 268:
        ALU_Dec(); break;
      case 269:
        MEM_WRITE(regWZ, regV); break;
      case 270:
        break;
      case 271:
        OP_FETCH; break;
      case 272: // ld (hl),N 10(4,3,3) ld (ix/iy+D),N 19(4,4,3,5,3)
        regWZ = regHLx; break;
      case 273:
        MEM_READ(regPC++); break;
      case 274:
        break;
      case 275:
        if (usingHL) { CYCLEINC(6); } break;
      case 276:
        regWZ += DISPLACE(cpuData); MEM_READ(regPC++); break; // overlap
      case 277:
        break;
      case 278:
        break;
      case 279:
        break;
      case 280:
        break;
      case 281:
        MEM_WRITE(regWZ, cpuData); break;
      case 282:
        break;
      case 283:
        OP_FETCH; break;
      case 284: // scf 4(4)
        regF = FFLAG(~(HF | NF)) | CF; OP_FETCH; break;
      case 285: // jr c,D 7(4,3) 12(4,3,5)
        break;
      case 286:
        MEM_READ(regPC++); break;
      case 287:
        break;
      case 288:
        if (!FFLAG(CF)) { OP_FETCH; } break;
      case 289:
        regPC += DISPLACE(cpuData); break;
      case 290:
        break;
      case 291:
        break;
      case 292:
        break;
      case 293:
        OP_FETCH; break;
      case 294: // add hl,sp 11(4,4,3) add ix/iy,sp 15(4,4,4,3)
        break;
      case 295:
        regQ = ZF; break;
      case 296:
        ALU_Add(regLx, regSPl, 0); break;
      case 297:
        regLx = regV; break;
      case 298:
        break;
      case 299:
        ALU_Add(regHx, regSPh, QFLAG(CF)); break;
      case 300:
        regHx = regV; regF = FFLAG(SF | ZF | VF) | QFLAG(HF | NF | CF); break;
      case 301:
        OP_FETCH; break;
      case 302: // ld a,(NN) 13(4,3,3,3)
        break;
      case 303:
        MEM_READ(regPC++); break;
      case 304:
        break;
      case 305:
        regZ = cpuData; break;
      case 306:
        MEM_READ(regPC++); break;
      case 307:
        break;
      case 308:
        regW = cpuData; break;
      case 309:
        MEM_READ(regWZ); break;
      case 310:
        break;
      case 311:
        regA = cpuData; OP_FETCH; break;
      case 312: // dec sp 6(6)
        break;
      case 313:
        regSP--; break;
      case 314:
        OP_FETCH; break;
      case 315: // inc a 4(4)
        regV = regA; ALU_Inc(); regA = regV; OP_FETCH; break;
      case 316: // dec a 4(4)
        regV = regA; ALU_Dec(); regA = regV; OP_FETCH; break;
      case 317: // ld a,N 7(4,3)
        break;
      case 318:
        MEM_READ(regPC++); break;
      case 319:
        break;
      case 320:
        regA = cpuData; OP_FETCH; break;
      case 321: // ccf 4(4)
        regF = (FFLAG(~(HF | NF)) | (FFLAG(CF) << 4 /* HF <- CF */)) ^ CF; OP_FETCH; break;
      case 322: // ld b,b 4(4)
        regB = regB; OP_FETCH; break;
      case 323: // ld b,c 4(4)
        regB = regC; OP_FETCH; break;
      case 324: // ld b,d 4(4)
        regB = regD; OP_FETCH; break;
      case 325: // ld b,e 4(4)
        regB = regE; OP_FETCH; break;
      case 326: // ld b,h 4(4) ld b,ixh/iyh 8(4,4)
        regB = regHx; OP_FETCH; break;
      case 327: // ld b,l 4(4) ld b,ixl/iyl 8(4,4)
        regB = regLx; OP_FETCH; break;
      case 328: // ld b,(hl) 7(4,3) ld b,(ix/iy+D) 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 329:
        MEM_READ(regPC++); break;
      case 330:
        break;
      case 331:
        break;
      case 332:
        regWZ += DISPLACE(cpuData); break;
      case 333:
        break;
      case 334:
        break;
      case 335:
        break;
      case 336:
        break;
      case 337:
        MEM_READ(regWZ); break;
      case 338:
        break;
      case 339:
        regB = cpuData; OP_FETCH; break;
      case 340: // ld b,a 4(4)
        regB = regA; OP_FETCH; break;
      case 341: // ld c,b 4(4)
        regC = regB; OP_FETCH; break;
      case 342: // ld c,c 4(4)
        regC = regC; OP_FETCH; break;
      case 343: // ld c,d 4(4)
        regC = regD; OP_FETCH; break;
      case 344: // ld c,e 4(4)
        regC = regE; OP_FETCH; break;
      case 345: // ld c,h 4(4) ld c,ixh/iyh 8(4,4)
        regC = regHx; OP_FETCH; break;
      case 346: // ld c,l 4(4) ld c,ixl/iyl 8(4,4)
        regC = regLx; OP_FETCH; break;
      case 347: // ld c,(hl) 7(4,3) ld c,(ix/iy+D) 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 348:
        MEM_READ(regPC++); break;
      case 349:
        break;
      case 350:
        break;
      case 351:
        regWZ += DISPLACE(cpuData); break;
      case 352:
        break;
      case 353:
        break;
      case 354:
        break;
      case 355:
        break;
      case 356:
        MEM_READ(regWZ); break;
      case 357:
        break;
      case 358:
        regC = cpuData; OP_FETCH; break;
      case 359: // ld c,a 4(4)
        regC = regA; OP_FETCH; break;
      case 360: // ld d,b 4(4)
        regD = regB; OP_FETCH; break;
      case 361: // ld d,c 4(4)
        regD = regC; OP_FETCH; break;
      case 362: // ld d,d 4(4)
        regD = regD; OP_FETCH; break;
      case 363: // ld d,e 4(4)
        regD = regE; OP_FETCH; break;
      case 364: // ld d,h 4(4) ld d,ixh/iyh 8(4,4)
        regD = regHx; OP_FETCH; break;
      case 365: // ld d,l 4(4) ld d,ixl/iyl 8(4,4)
        regD = regLx; OP_FETCH; break;
      case 366: // ld d,(hl) 7(4,3) ld d,(ix/iy+D) 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 367:
        MEM_READ(regPC++); break;
      case 368:
        break;
      case 369:
        break;
      case 370:
        regWZ += DISPLACE(cpuData); break;
      case 371:
        break;
      case 372:
        break;
      case 373:
        break;
      case 374:
        break;
      case 375:
        MEM_READ(regWZ); break;
      case 376:
        break;
      case 377:
        regD = cpuData; OP_FETCH; break;
      case 378: // ld d,a 4(4)
        regD = regA; OP_FETCH; break;
      case 379: // ld e,b 4(4)
        regE = regB; OP_FETCH; break;
      case 380: // ld e,c 4(4)
        regE = regC; OP_FETCH; break;
      case 381: // ld e,d 4(4)
        regE = regD; OP_FETCH; break;
      case 382: // ld e,e 4(4)
        regE = regE; OP_FETCH; break;
      case 383: // ld e,h 4(4) ld e,ixh/iyh 8(4,4)
        regE = regHx; OP_FETCH; break;
      case 384: // ld e,l 4(4) ld e,ixl/iyl 8(4,4)
        regE = regLx; OP_FETCH; break;
      case 385: // ld e,(hl) 7(4,3) ld e,(ix/iy+D) 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 386:
        MEM_READ(regPC++); break;
      case 387:
        break;
      case 388:
        break;
      case 389:
        regWZ += DISPLACE(cpuData); break;
      case 390:
        break;
      case 391:
        break;
      case 392:
        break;
      case 393:
        break;
      case 394:
        MEM_READ(regWZ); break;
      case 395:
        break;
      case 396:
        regE = cpuData; OP_FETCH; break;
      case 397: // ld e,a 4(4)
        regE = regA; OP_FETCH; break;
      case 398: // ld h,b 4(4)
        regHx = regB; OP_FETCH; break;
      case 399: // ld h,c 4(4)
        regHx = regC; OP_FETCH; break;
      case 400: // ld h,d 4(4)
        regHx = regD; OP_FETCH; break;
      case 401: // ld h,e 4(4)
        regHx = regE; OP_FETCH; break;
      case 402: // ld h,h 4(4) ld ixh/iyh,ixh/iyh 8(4,4)
        regHx = regHx; OP_FETCH; break;
      case 403: // ld h,l 4(4) ld ixh/iyh,ixl/iyl 8(4,4)
        regHx = regLx; OP_FETCH; break;
      case 404: // ld h,(hl) 7(4,3) ld h,(ix/iy+D) 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 405:
        MEM_READ(regPC++); break;
      case 406:
        break;
      case 407:
        break;
      case 408:
        regWZ += DISPLACE(cpuData); break;
      case 409:
        break;
      case 410:
        break;
      case 411:
        break;
      case 412:
        break;
      case 413:
        MEM_READ(regWZ); break;
      case 414:
        break;
      case 415:
        regH = cpuData; OP_FETCH; break;
      case 416: // ld h,a 4(4)
        regHx = regA; OP_FETCH; break;
      case 417: // ld l,b 4(4)
        regLx = regB; OP_FETCH; break;
      case 418: // ld l,c 4(4)
        regLx = regC; OP_FETCH; break;
      case 419: // ld l,d 4(4)
        regLx = regD; OP_FETCH; break;
      case 420: // ld l,e 4(4)
        regLx = regE; OP_FETCH; break;
      case 421: // ld l,h 4(4) ld ixl/iyl,ixh/iyh 8(4,4)
        regLx = regHx; OP_FETCH; break;
      case 422: // ld l,l 4(4) ld ixl/iyl,ixl/iyl 8(4,4)
        regLx = regLx; OP_FETCH; break;
      case 423: // ld l,(hl) 7(4,3) ld l,(ix/iy+D) 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 424:
        MEM_READ(regPC++); break;
      case 425:
        break;
      case 426:
        break;
      case 427:
        regWZ += DISPLACE(cpuData); break;
      case 428:
        break;
      case 429:
        break;
      case 430:
        break;
      case 431:
        break;
      case 432:
        MEM_READ(regWZ); break;
      case 433:
        break;
      case 434:
        regL = cpuData; OP_FETCH; break;
      case 435: // ld l,a 4(4)
        regLx = regA; OP_FETCH; break;
      case 436: // ld (hl),b 7(4,3) ld (ix/iy+D),b 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 437:
        MEM_READ(regPC++); break;
      case 438:
        break;
      case 439:
        break;
      case 440:
        regWZ += DISPLACE(cpuData); break;
      case 441:
        break;
      case 442:
        break;
      case 443:
        break;
      case 444:
        break;
      case 445:
        MEM_WRITE(regWZ, regB); break;
      case 446:
        break;
      case 447:
        OP_FETCH; break;
      case 448: // ld (hl),c 7(4,3) ld (ix/iy+D),c 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 449:
        MEM_READ(regPC++); break;
      case 450:
        break;
      case 451:
        break;
      case 452:
        regWZ += DISPLACE(cpuData); break;
      case 453:
        break;
      case 454:
        break;
      case 455:
        break;
      case 456:
        break;
      case 457:
        MEM_WRITE(regWZ, regC); break;
      case 458:
        break;
      case 459:
        OP_FETCH; break;
      case 460: // ld (hl),d 7(4,3) ld (ix/iy+D),d 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 461:
        MEM_READ(regPC++); break;
      case 462:
        break;
      case 463:
        break;
      case 464:
        regWZ += DISPLACE(cpuData); break;
      case 465:
        break;
      case 466:
        break;
      case 467:
        break;
      case 468:
        break;
      case 469:
        MEM_WRITE(regWZ, regD); break;
      case 470:
        break;
      case 471:
        OP_FETCH; break;
      case 472: // ld (hl),e 7(4,3) ld (ix/iy+D),e 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 473:
        MEM_READ(regPC++); break;
      case 474:
        break;
      case 475:
        break;
      case 476:
        regWZ += DISPLACE(cpuData); break;
      case 477:
        break;
      case 478:
        break;
      case 479:
        break;
      case 480:
        break;
      case 481:
        MEM_WRITE(regWZ, regE); break;
      case 482:
        break;
      case 483:
        OP_FETCH; break;
      case 484: // ld (hl),h 7(4,3) ld (ix/iy+D),h 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 485:
        MEM_READ(regPC++); break;
      case 486:
        break;
      case 487:
        break;
      case 488:
        regWZ += DISPLACE(cpuData); break;
      case 489:
        break;
      case 490:
        break;
      case 491:
        break;
      case 492:
        break;
      case 493:
        MEM_WRITE(regWZ, regH); break;
      case 494:
        break;
      case 495:
        OP_FETCH; break;
      case 496: // ld (hl),l 7(4,3) ld (ix/iy+D),l 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 497:
        MEM_READ(regPC++); break;
      case 498:
        break;
      case 499:
        break;
      case 500:
        regWZ += DISPLACE(cpuData); break;
      case 501:
        break;
      case 502:
        break;
      case 503:
        break;
      case 504:
        break;
      case 505:
        MEM_WRITE(regWZ, regL); break;
      case 506:
        break;
      case 507:
        OP_FETCH; break;
      case 508: // halt 4(4)
        CPU_HALT = 1; regPC--; OP_FETCH; break;
      case 509: // ld (hl),a 7(4,3) ld (ix/iy+D),a 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 510:
        MEM_READ(regPC++); break;
      case 511:
        break;
      case 512:
        break;
      case 513:
        regWZ += DISPLACE(cpuData); break;
      case 514:
        break;
      case 515:
        break;
      case 516:
        break;
      case 517:
        break;
      case 518:
        MEM_WRITE(regWZ, regA); break;
      case 519:
        break;
      case 520:
        OP_FETCH; break;
      case 521: // ld a,b 4(4)
        regA = regB; OP_FETCH; break;
      case 522: // ld a,c 4(4)
        regA = regC; OP_FETCH; break;
      case 523: // ld a,d 4(4)
        regA = regD; OP_FETCH; break;
      case 524: // ld a,e 4(4)
        regA = regE; OP_FETCH; break;
      case 525: // ld a,h 4(4) ld a,ixh/iyh 8(4,4)
        regA = regHx; OP_FETCH; break;
      case 526: // ld a,l 4(4) ld a,ixl/iyl 8(4,4)
        regA = regLx; OP_FETCH; break;
      case 527: // ld a,(hl) 7(4,3) ld a,(ix/iy+D) 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 528:
        MEM_READ(regPC++); break;
      case 529:
        break;
      case 530:
        break;
      case 531:
        regWZ += DISPLACE(cpuData); break;
      case 532:
        break;
      case 533:
        break;
      case 534:
        break;
      case 535:
        break;
      case 536:
        MEM_READ(regWZ); break;
      case 537:
        break;
      case 538:
        regA = cpuData; OP_FETCH; break;
      case 539: // ld a,a 4(4)
        regA = regA; OP_FETCH; break;
      case 540: // add a,b 4(4)
        regV = regB; ALU_ADD; OP_FETCH; break;
      case 541: // add a,c 4(4)
        regV = regC; ALU_ADD; OP_FETCH; break;
      case 542: // add a,d 4(4)
        regV = regD; ALU_ADD; OP_FETCH; break;
      case 543: // add a,e 4(4)
        regV = regE; ALU_ADD; OP_FETCH; break;
      case 544: // add a,h 4(4) add a,ixh/iyh 8(4,4)
        regV = regHx; ALU_ADD; OP_FETCH; break;
      case 545: // add a,l 4(4) add a,ixl/iyl 8(4,4)
        regV = regLx; ALU_ADD; OP_FETCH; break;
      case 546: // add a,(hl) 7(4,3) add a,(ix/iy+D) 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 547:
        MEM_READ(regPC++); break;
      case 548:
        break;
      case 549:
        break;
      case 550:
        regWZ += DISPLACE(cpuData); break;
      case 551:
        break;
      case 552:
        break;
      case 553:
        break;
      case 554:
        break;
      case 555:
        MEM_READ(regWZ); break;
      case 556:
        break;
      case 557:
        regV = cpuData; ALU_ADD; OP_FETCH; break;
      case 558: // add a,a 4(4)
        regV = regA; ALU_ADD; OP_FETCH; break;
      case 559: // adc a,b 4(4)
        regV = regB; ALU_ADC; OP_FETCH; break;
      case 560: // adc a,c 4(4)
        regV = regC; ALU_ADC; OP_FETCH; break;
      case 561: // adc a,d 4(4)
        regV = regD; ALU_ADC; OP_FETCH; break;
      case 562: // adc a,e 4(4)
        regV = regE; ALU_ADC; OP_FETCH; break;
      case 563: // adc a,h 4(4) adc a,ixh/iyh 8(4,4)
        regV = regHx; ALU_ADC; OP_FETCH; break;
      case 564: // adc a,l 4(4) adc a,ixl/iyl 8(4,4)
        regV = regLx; ALU_ADC; OP_FETCH; break;
      case 565: // adc a,(hl) 7(4,3) adc a,(ix/iy+D) 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 566:
        MEM_READ(regPC++); break;
      case 567:
        break;
      case 568:
        break;
      case 569:
        regWZ += DISPLACE(cpuData); break;
      case 570:
        break;
      case 571:
        break;
      case 572:
        break;
      case 573:
        break;
      case 574:
        MEM_READ(regWZ); break;
      case 575:
        break;
      case 576:
        regV = cpuData; ALU_ADC; OP_FETCH; break;
      case 577: // adc a,a 4(4)
        regV = regA; ALU_ADC; OP_FETCH; break;
      case 578: // sub b 4(4)
        regV = regB; ALU_SUB; OP_FETCH; break;
      case 579: // sub c 4(4)
        regV = regC; ALU_SUB; OP_FETCH; break;
      case 580: // sub d 4(4)
        regV = regD; ALU_SUB; OP_FETCH; break;
      case 581: // sub e 4(4)
        regV = regE; ALU_SUB; OP_FETCH; break;
      case 582: // sub h 4(4) sub ixh/iyh 8(4,4)
        regV = regHx; ALU_SUB; OP_FETCH; break;
      case 583: // sub l 4(4) sub ixl/iyl 8(4,4)
        regV = regLx; ALU_SUB; OP_FETCH; break;
      case 584: // sub (hl) 7(4,3) sub (ix/iy+D) 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 585:
        MEM_READ(regPC++); break;
      case 586:
        break;
      case 587:
        break;
      case 588:
        regWZ += DISPLACE(cpuData); break;
      case 589:
        break;
      case 590:
        break;
      case 591:
        break;
      case 592:
        break;
      case 593:
        MEM_READ(regWZ); break;
      case 594:
        break;
      case 595:
        regV = cpuData; ALU_SUB; OP_FETCH; break;
      case 596: // sub a 4(4)
        regV = regA; ALU_SUB; OP_FETCH; break;
      case 597: // sbc a,b 4(4)
        regV = regB; ALU_SBC; OP_FETCH; break;
      case 598: // sbc a,c 4(4)
        regV = regC; ALU_SBC; OP_FETCH; break;
      case 599: // sbc a,d 4(4)
        regV = regD; ALU_SBC; OP_FETCH; break;
      case 600: // sbc a,e 4(4)
        regV = regE; ALU_SBC; OP_FETCH; break;
      case 601: // sbc a,h 4(4) sbc a,ixh/iyh 8(4,4)
        regV = regHx; ALU_SBC; OP_FETCH; break;
      case 602: // sbc a,l 4(4) sbc a,ixl/iyl 8(4,4)
        regV = regLx; ALU_SBC; OP_FETCH; break;
      case 603: // sbc a,(hl) 7(4,3) sbc a,(ix/iy+D) 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 604:
        MEM_READ(regPC++); break;
      case 605:
        break;
      case 606:
        break;
      case 607:
        regWZ += DISPLACE(cpuData); break;
      case 608:
        break;
      case 609:
        break;
      case 610:
        break;
      case 611:
        break;
      case 612:
        MEM_READ(regWZ); break;
      case 613:
        break;
      case 614:
        regV = cpuData; ALU_SBC; OP_FETCH; break;
      case 615: // sbc a,a 4(4)
        regV = regA; ALU_SBC; OP_FETCH; break;
      case 616: // and b 4(4)
        regV = regB; ALU_And(); OP_FETCH; break;
      case 617: // and c 4(4)
        regV = regC; ALU_And(); OP_FETCH; break;
      case 618: // and d 4(4)
        regV = regD; ALU_And(); OP_FETCH; break;
      case 619: // and e 4(4)
        regV = regE; ALU_And(); OP_FETCH; break;
      case 620: // and h 4(4) and ixh/iyh 8(4,4)
        regV = regHx; ALU_And(); OP_FETCH; break;
      case 621: // and l 4(4) and ixl/iyl 8(4,4)
        regV = regLx; ALU_And(); OP_FETCH; break;
      case 622: // and (hl) 7(4,3) and (ix/iy+D) 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 623:
        MEM_READ(regPC++); break;
      case 624:
        break;
      case 625:
        break;
      case 626:
        regWZ += DISPLACE(cpuData); break;
      case 627:
        break;
      case 628:
        break;
      case 629:
        break;
      case 630:
        break;
      case 631:
        MEM_READ(regWZ); break;
      case 632:
        break;
      case 633:
        regV = cpuData; ALU_And(); OP_FETCH; break;
      case 634: // and a 4(4)
        regV = regA; ALU_And(); OP_FETCH; break;
      case 635: // xor b 4(4)
        regV = regB; ALU_Xor(); OP_FETCH; break;
      case 636: // xor c 4(4)
        regV = regC; ALU_Xor(); OP_FETCH; break;
      case 637: // xor d 4(4)
        regV = regD; ALU_Xor(); OP_FETCH; break;
      case 638: // xor e 4(4)
        regV = regE; ALU_Xor(); OP_FETCH; break;
      case 639: // xor h 4(4) xor ixh/iyh 8(4,4)
        regV = regHx; ALU_Xor(); OP_FETCH; break;
      case 640: // xor l 4(4) xor ixl/iyl 8(4,4)
        regV = regLx; ALU_Xor(); OP_FETCH; break;
      case 641: // xor (hl) 7(4,3) xor (ix/iy+D) 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 642:
        MEM_READ(regPC++); break;
      case 643:
        break;
      case 644:
        break;
      case 645:
        regWZ += DISPLACE(cpuData); break;
      case 646:
        break;
      case 647:
        break;
      case 648:
        break;
      case 649:
        break;
      case 650:
        MEM_READ(regWZ); break;
      case 651:
        break;
      case 652:
        regV = cpuData; ALU_Xor(); OP_FETCH; break;
      case 653: // xor a 4(4)
        regV = regA; ALU_Xor(); OP_FETCH; break;
      case 654: // or b 4(4)
        regV = regB; ALU_Or(); OP_FETCH; break;
      case 655: // or c 4(4)
        regV = regC; ALU_Or(); OP_FETCH; break;
      case 656: // or d 4(4)
        regV = regD; ALU_Or(); OP_FETCH; break;
      case 657: // or e 4(4)
        regV = regE; ALU_Or(); OP_FETCH; break;
      case 658: // or h 4(4) or ixh/iyh 8(4,4)
        regV = regHx; ALU_Or(); OP_FETCH; break;
      case 659: // or l 4(4) or ixl/iyl 8(4,4)
        regV = regLx; ALU_Or(); OP_FETCH; break;
      case 660: // or (hl) 7(4,3) or (ix/iy+D) 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 661:
        MEM_READ(regPC++); break;
      case 662:
        break;
      case 663:
        break;
      case 664:
        regWZ += DISPLACE(cpuData); break;
      case 665:
        break;
      case 666:
        break;
      case 667:
        break;
      case 668:
        break;
      case 669:
        MEM_READ(regWZ); break;
      case 670:
        break;
      case 671:
        regV = cpuData; ALU_Or(); OP_FETCH; break;
      case 672: // or a 4(4)
        regV = regA; ALU_Or(); OP_FETCH; break;
      case 673: // cp b 4(4)
        regV = regB; ALU_Cp(); OP_FETCH; break;
      case 674: // cp c 4(4)
        regV = regC; ALU_Cp(); OP_FETCH; break;
      case 675: // cp d 4(4)
        regV = regD; ALU_Cp(); OP_FETCH; break;
      case 676: // cp e 4(4)
        regV = regE; ALU_Cp(); OP_FETCH; break;
      case 677: // cp h 4(4) cp ixh/iyh 8(4,4)
        regV = regHx; ALU_Cp(); OP_FETCH; break;
      case 678: // cp l 4(4) cp ixl/iyl 8(4,4)
        regV = regLx; ALU_Cp(); OP_FETCH; break;
      case 679: // cp (hl) 7(4,3) cp (ix/iy+D) 19(4,4,3,5,3)
        regWZ = regHLx; if (usingHL) { CYCLEINC(9); } break;
      case 680:
        MEM_READ(regPC++); break;
      case 681:
        break;
      case 682:
        break;
      case 683:
        regWZ += DISPLACE(cpuData); break;
      case 684:
        break;
      case 685:
        break;
      case 686:
        break;
      case 687:
        break;
      case 688:
        MEM_READ(regWZ); break;
      case 689:
        break;
      case 690:
        regV = cpuData; ALU_Cp(); OP_FETCH; break;
      case 691: // cp a 4(4)
        regV = regA; ALU_Cp(); OP_FETCH; break;
      case 692: // ret nz 5(5) 11(5,3,3)
        break;
      case 693:
        if (FFLAG(ZF)) { OP_FETCH; } break;
      case 694:
        MEM_READ(regSP); break;
      case 695:
        regSP++; break;
      case 696:
        regPCl = cpuData; break;
      case 697:
        MEM_READ(regSP); break;
      case 698:
        regSP++; break;
      case 699:
        regPCh = cpuData; OP_FETCH; break;
      case 700: // pop bc 10(4,3,3)
        break;
      case 701:
        MEM_READ(regSP); break;
      case 702:
        regSP++; break;
      case 703:
        regC = cpuData; break;
      case 704:
        MEM_READ(regSP); break;
      case 705:
        regSP++; break;
      case 706:
        regB = cpuData; OP_FETCH; break;
      case 707: // jp nz,NN 10(4,3,3)
        break;
      case 708:
        MEM_READ(regPC++); break;
      case 709:
        break;
      case 710:
        regZ = cpuData; break;
      case 711:
        MEM_READ(regPC++); break;
      case 712:
        break;
      case 713:
        regW = cpuData; if (!FFLAG(ZF)) { regPC = regWZ; } OP_FETCH; break;
      case 714: // jp NN 10(4,3,3)
        break;
      case 715:
        MEM_READ(regPC++); break;
      case 716:
        break;
      case 717:
        regZ = cpuData; break;
      case 718:
        MEM_READ(regPC++); break;
      case 719:
        break;
      case 720:
        regW = cpuData; regPC = regWZ; OP_FETCH; break;
      case 721: // call nz,NN 10(4,3,3) 17(4,3,4,3,3)
        break;
      case 722:
        MEM_READ(regPC++); break;
      case 723:
        break;
      case 724:
        regZ = cpuData; break;
      case 725:
        MEM_READ(regPC++); break;
      case 726:
        break;
      case 727:
        regW = cpuData; if (FFLAG(ZF)) { OP_FETCH; } break;
      case 728:
        regSP--; break;
      case 729:
        MEM_WRITE(regSP, regPCh); break;
      case 730:
        regSP--; break;
      case 731:
        break;
      case 732:
        MEM_WRITE(regSP, regPCl); break;
      case 733:
        break;
      case 734:
        regPC = regWZ; OP_FETCH; break;
      case 735: // push bc 11(5,3,3)
        regWZ = regBC; break;
      case 736:
        regSP--; break;
      case 737:
        MEM_WRITE(regSP, regW); break;
      case 738:
        regSP--; break;
      case 739:
        break;
      case 740:
        MEM_WRITE(regSP, regZ); break;
      case 741:
        break;
      case 742:
        OP_FETCH; break;
      case 743: // add a,N 7(4,3)
        break;
      case 744:
        MEM_READ(regPC++); break;
      case 745:
        break;
      case 746:
        regV = cpuData; ALU_ADD; OP_FETCH; break;
      case 747: // rst 00h 11(5,3,3)
        break;
      case 748:
        regSP--; break;
      case 749:
        MEM_WRITE(regSP, regPCh); break;
      case 750:
        regSP--; break;
      case 751:
        break;
      case 752:
        MEM_WRITE(regSP, regPCl); break;
      case 753:
        break;
      case 754:
        regPC = 0x00; OP_FETCH; break;
      case 755: // ret z 5(5) 11(5,3,3)
        break;
      case 756:
        if (!FFLAG(ZF)) { OP_FETCH; } break;
      case 757:
        MEM_READ(regSP); break;
      case 758:
        regSP++; break;
      case 759:
        regPCl = cpuData; break;
      case 760:
        MEM_READ(regSP); break;
      case 761:
        regSP++; break;
      case 762:
        regPCh = cpuData; OP_FETCH; break;
      case 763: // ret 10(4,3,3)
        break;
      case 764:
        MEM_READ(regSP); break;
      case 765:
        regSP++; break;
      case 766:
        regPCl = cpuData; break;
      case 767:
        MEM_READ(regSP); break;
      case 768:
        regSP++; break;
      case 769:
        regPCh = cpuData; OP_FETCH; break;
      case 770: // jp z,NN 10(4,3,3)
        break;
      case 771:
        MEM_READ(regPC++); break;
      case 772:
        break;
      case 773:
        regZ = cpuData; break;
      case 774:
        MEM_READ(regPC++); break;
      case 775:
        break;
      case 776:
        regW = cpuData; if (FFLAG(ZF)) { regPC = regWZ; } OP_FETCH; break;
      case 777: // CB 4(4 ...) 19(4,4,3,5,3 ...)
        regWZ = regHLx; if (usingHL) { M1_FETCH(2); } break;
      case 778: // IX or IY
        MEM_READ(regPC++); break;
      case 779:
        break;
      case 780:
        regWZ += DISPLACE(cpuData); break;
      case 781:
        MEM_READ(regWZ); break;
      case 782:
        break;
      case 783:
        regV = cpuData; break; // regV required for final part of instruction
      case 784:
        break;
      case 785:
        break;
      case 786:
        MEM_READ(regPC++); break;
      case 787:
        break;
      case 788:
        cpuIR = cpuData; OPCYCLE(cpuOp[3][cpuIR]); break;
      case 789: // call z,NN 10(4,3,3) 17(4,3,4,3,3)
        break;
      case 790:
        MEM_READ(regPC++); break;
      case 791:
        break;
      case 792:
        regZ = cpuData; break;
      case 793:
        MEM_READ(regPC++); break;
      case 794:
        break;
      case 795:
        regW = cpuData; if (!FFLAG(ZF)) { OP_FETCH; } break;
      case 796:
        regSP--; break;
      case 797:
        MEM_WRITE(regSP, regPCh); break;
      case 798:
        regSP--; break;
      case 799:
        break;
      case 800:
        MEM_WRITE(regSP, regPCl); break;
      case 801:
        break;
      case 802:
        regPC = regWZ; OP_FETCH; break;
      case 803: // call NN 17(4,3,4,3,3)
        break;
      case 804:
        MEM_READ(regPC++); break;
      case 805:
        break;
      case 806:
        regZ = cpuData; break;
      case 807:
        MEM_READ(regPC++); break;
      case 808:
        break;
      case 809:
        regW = cpuData; break;
      case 810:
        regSP--; break;
      case 811:
        MEM_WRITE(regSP, regPCh); break;
      case 812:
        regSP--; break;
      case 813:
        break;
      case 814:
        MEM_WRITE(regSP, regPCl); break;
      case 815:
        break;
      case 816:
        regPC = regWZ; OP_FETCH; break;
      case 817: // adc a,N 7(4,3)
        break;
      case 818:
        MEM_READ(regPC++); break;
      case 819:
        break;
      case 820:
        regV = cpuData; ALU_ADC; OP_FETCH; break;
      case 821: // rst 08h 11(5,3,3)
        break;
      case 822:
        regSP--; break;
      case 823:
        MEM_WRITE(regSP, regPCh); break;
      case 824:
        regSP--; break;
      case 825:
        break;
      case 826:
        MEM_WRITE(regSP, regPCl); break;
      case 827:
        break;
      case 828:
        regPC = 0x08; OP_FETCH; break;
      case 829: // ret nc 5(5) 11(5,3,3)
        break;
      case 830:
        if (FFLAG(CF)) { OP_FETCH; } break;
      case 831:
        MEM_READ(regSP); break;
      case 832:
        regSP++; break;
      case 833:
        regPCl = cpuData; break;
      case 834:
        MEM_READ(regSP); break;
      case 835:
        regSP++; break;
      case 836:
        regPCh = cpuData; OP_FETCH; break;
      case 837: // pop de 10(4,3,3)
        break;
      case 838:
        MEM_READ(regSP); break;
      case 839:
        regSP++; break;
      case 840:
        regE = cpuData; break;
      case 841:
        MEM_READ(regSP); break;
      case 842:
        regSP++; break;
      case 843:
        regD = cpuData; OP_FETCH; break;
      case 844: // jp nc,NN 10(4,3,3)
        break;
      case 845:
        MEM_READ(regPC++); break;
      case 846:
        break;
      case 847:
        regZ = cpuData; break;
      case 848:
        MEM_READ(regPC++); break;
      case 849:
        break;
      case 850:
        regW = cpuData; if (!FFLAG(CF)) { regPC = regWZ; } OP_FETCH; break;
      case 851: // out (N),a 11(4,3,4)
        break;
      case 852:
        MEM_READ(regPC++); break;
      case 853:
        break;
      case 854:
        regV = cpuData; break;
      case 855:
        BUS_ADD_DATA((regA << 8) | regV, regA); break;
      case 856:
        IO_WRITE; break;
      case 857:
        break;
      case 858:
        OP_FETCH; break;
      case 859: // call nc,NN 10(4,3,3) 17(4,3,4,3,3)
        break;
      case 860:
        MEM_READ(regPC++); break;
      case 861:
        break;
      case 862:
        regZ = cpuData; break;
      case 863:
        MEM_READ(regPC++); break;
      case 864:
        break;
      case 865:
        regW = cpuData; if (FFLAG(CF)) { OP_FETCH; } break;
      case 866:
        regSP--; break;
      case 867:
        MEM_WRITE(regSP, regPCh); break;
      case 868:
        regSP--; break;
      case 869:
        break;
      case 870:
        MEM_WRITE(regSP, regPCl); break;
      case 871:
        break;
      case 872:
        regPC = regWZ; OP_FETCH; break;
      case 873: // push de 11(5,3,3)
        regWZ = regDE; break;
      case 874:
        regSP--; break;
      case 875:
        MEM_WRITE(regSP, regW); break;
      case 876:
        regSP--; break;
      case 877:
        break;
      case 878:
        MEM_WRITE(regSP, regZ); break;
      case 879:
        break;
      case 880:
        OP_FETCH; break;
      case 881: // sub N 7(4,3)
        break;
      case 882:
        MEM_READ(regPC++); break;
      case 883:
        break;
      case 884:
        regV = cpuData; ALU_SUB; OP_FETCH; break;
      case 885: // rst 10h 11(5,3,3)
        break;
      case 886:
        regSP--; break;
      case 887:
        MEM_WRITE(regSP, regPCh); break;
      case 888:
        regSP--; break;
      case 889:
        break;
      case 890:
        MEM_WRITE(regSP, regPCl); break;
      case 891:
        break;
      case 892:
        regPC = 0x10; OP_FETCH; break;
      case 893: // ret c 5(5) 11(5,3,3)
        break;
      case 894:
        if (!FFLAG(CF)) { OP_FETCH; } break;
      case 895:
        MEM_READ(regSP); break;
      case 896:
        regSP++; break;
      case 897:
        regPCl = cpuData; break;
      case 898:
        MEM_READ(regSP); break;
      case 899:
        regSP++; break;
      case 900:
        regPCh = cpuData; OP_FETCH; break;
      case 901: // exx 4(4)
        regFF[1] ^= 1; regBCu = &regMain[regFF[1]]; regDEu = REG_DE; regHLu = REG_HL; OP_FETCH; break;
      case 902: // jp c,NN 10(4,3,3)
        break;
      case 903:
        MEM_READ(regPC++); break;
      case 904:
        break;
      case 905:
        regZ = cpuData; break;
      case 906:
        MEM_READ(regPC++); break;
      case 907:
        break;
      case 908:
        regW = cpuData; if (FFLAG(CF)) { regPC = regWZ; } OP_FETCH; break;
      case 909: // in a,(N) 11(4,3,4)
        break;
      case 910:
        MEM_READ(regPC++); break;
      case 911:
        break;
      case 912:
        regV = cpuData; break;
      case 913:
        BUS_ADD((regA << 8) | regV); break;
      case 914:
        IO_READ; break;
      case 915:
        break;
      case 916:
        regA = cpuData; OP_FETCH; break;
      case 917: // call c,NN 10(4,3,3) 17(4,3,4,3,3)
        break;
      case 918:
        MEM_READ(regPC++); break;
      case 919:
        break;
      case 920:
        regZ = cpuData; break;
      case 921:
        MEM_READ(regPC++); break;
      case 922:
        break;
      case 923:
        regW = cpuData; if (!FFLAG(CF)) { OP_FETCH; } break;
      case 924:
        regSP--; break;
      case 925:
        MEM_WRITE(regSP, regPCh); break;
      case 926:
        regSP--; break;
      case 927:
        break;
      case 928:
        MEM_WRITE(regSP, regPCl); break;
      case 929:
        break;
      case 930:
        regPC = regWZ; OP_FETCH; break;
      case 931: // DD (4)
        USE_IX; M1_FETCH(0); break;
      case 932: // sbc a,N 7(4,3)
        break;
      case 933:
        MEM_READ(regPC++); break;
      case 934:
        break;
      case 935:
        regV = cpuData; ALU_SBC; OP_FETCH; break;
      case 936: // rst 18h 11(5,3,3)
        break;
      case 937:
        regSP--; break;
      case 938:
        MEM_WRITE(regSP, regPCh); break;
      case 939:
        regSP--; break;
      case 940:
        break;
      case 941:
        MEM_WRITE(regSP, regPCl); break;
      case 942:
        break;
      case 943:
        regPC = 0x18; OP_FETCH; break;
      case 944: // ret po 5(5) 11(5,3,3)
        break;
      case 945:
        if (FFLAG(PF)) { OP_FETCH; } break;
      case 946:
        MEM_READ(regSP); break;
      case 947:
        regSP++; break;
      case 948:
        regPCl = cpuData; break;
      case 949:
        MEM_READ(regSP); break;
      case 950:
        regSP++; break;
      case 951:
        regPCh = cpuData; OP_FETCH; break;
      case 952: // pop hl 10(4,3,3) pop ix/iy 14(4,4,3,3)
        break;
      case 953:
        MEM_READ(regSP); break;
      case 954:
        regSP++; break;
      case 955:
        regLx = cpuData; break;
      case 956:
        MEM_READ(regSP); break;
      case 957:
        regSP++; break;
      case 958:
        regHx = cpuData; OP_FETCH; break;
      case 959: // jp po,NN 10(4,3,3)
        break;
      case 960:
        MEM_READ(regPC++); break;
      case 961:
        break;
      case 962:
        regZ = cpuData; break;
      case 963:
        MEM_READ(regPC++); break;
      case 964:
        break;
      case 965:
        regW = cpuData; if (!FFLAG(PF)) { regPC = regWZ; } OP_FETCH; break;
      case 966: // ex (sp),hl 19(4,3,4,3,5) ex (sp),ix/iy 23(4,4,3,4,3,5)
        break;
      case 967:
        MEM_READ(regSP); break;
      case 968:
        break;
      case 969:
        regZ = cpuData; break;
      case 970:
        MEM_READ(regSP + 1); break;
      case 971:
        break;
      case 972:
        regW = cpuData; break;
      case 973:
        break;
      case 974:
        MEM_WRITE(regSP, regLx); break;
      case 975:
        break;
      case 976:
        break;
      case 977:
        MEM_WRITE(regSP + 1, regHx); break;
      case 978:
        break;
      case 979:
        break;
      case 980:
        regHLx = regWZ; break;
      case 981:
        OP_FETCH; break;
      case 982: // call po,NN 10(4,3,3) 17(4,3,4,3,3)
        break;
      case 983:
        MEM_READ(regPC++); break;
      case 984:
        break;
      case 985:
        regZ = cpuData; break;
      case 986:
        MEM_READ(regPC++); break;
      case 987:
        break;
      case 988:
        regW = cpuData; if (FFLAG(PF)) { OP_FETCH; } break;
      case 989:
        regSP--; break;
      case 990:
        MEM_WRITE(regSP, regPCh); break;
      case 991:
        regSP--; break;
      case 992:
        break;
      case 993:
        MEM_WRITE(regSP, regPCl); break;
      case 994:
        break;
      case 995:
        regPC = regWZ; OP_FETCH; break;
      case 996: // push hl 11(5,3,3)
        regWZ = regHLx; break;
      case 997:
        regSP--; break;
      case 998:
        MEM_WRITE(regSP, regW); break;
      case 999:
        regSP--; break;
      case 1000:
        break;
      case 1001:
        MEM_WRITE(regSP, regZ); break;
      case 1002:
        break;
      case 1003:
        OP_FETCH; break;
      case 1004: // and N 7(4,3)
        break;
      case 1005:
        MEM_READ(regPC++); break;
      case 1006:
        break;
      case 1007:
        regV = cpuData; ALU_And(); OP_FETCH; break;
      case 1008: // rst 20h 11(5,3,3)
        break;
      case 1009:
        regSP--; break;
      case 1010:
        MEM_WRITE(regSP, regPCh); break;
      case 1011:
        regSP--; break;
      case 1012:
        break;
      case 1013:
        MEM_WRITE(regSP, regPCl); break;
      case 1014:
        break;
      case 1015:
        regPC = 0x20; OP_FETCH; break;
      case 1016: // ret pe 5(5) 11(5,3,3)
        break;
      case 1017:
        if (!FFLAG(PF)) { OP_FETCH; } break;
      case 1018:
        MEM_READ(regSP); break;
      case 1019:
        regSP++; break;
      case 1020:
        regPCl = cpuData; break;
      case 1021:
        MEM_READ(regSP); break;
      case 1022:
        regSP++; break;
      case 1023:
        regPCh = cpuData; OP_FETCH; break;
      case 1024: // jp (hl) 4(4) jp ix/iy 8(4,4) NOTE: should be: jp hl
        regPC = regHLx; OP_FETCH; break;
      case 1025: // jp pe,NN 10(4,3,3)
        break;
      case 1026:
        MEM_READ(regPC++); break;
      case 1027:
        break;
      case 1028:
        regZ = cpuData; break;
      case 1029:
        MEM_READ(regPC++); break;
      case 1030:
        break;
      case 1031:
        regW = cpuData; if (FFLAG(PF)) { regPC = regWZ; } OP_FETCH; break;
      case 1032: // ex de,hl 4(4)
        regFF[regFF[1]] ^= 2; regDEu = REG_DE; regHLu = REG_HL; OP_FETCH; break;
      case 1033: // call pe,NN 10(4,3,3) 17(4,3,4,3,3)
        break;
      case 1034:
        MEM_READ(regPC++); break;
      case 1035:
        break;
      case 1036:
        regZ = cpuData; break;
      case 1037:
        MEM_READ(regPC++); break;
      case 1038:
        break;
      case 1039:
        regW = cpuData; if (!FFLAG(PF)) { OP_FETCH; } break;
      case 1040:
        regSP--; break;
      case 1041:
        MEM_WRITE(regSP, regPCh); break;
      case 1042:
        regSP--; break;
      case 1043:
        break;
      case 1044:
        MEM_WRITE(regSP, regPCl); break;
      case 1045:
        break;
      case 1046:
        regPC = regWZ; OP_FETCH; break;
      case 1047: // ED
        USE_HL; M1_FETCH(1); break;
      case 1048: // xor N 7(4,3)
        break;
      case 1049:
        MEM_READ(regPC++); break;
      case 1050:
        break;
      case 1051:
        regV = cpuData; ALU_Xor(); OP_FETCH; break;
      case 1052: // rst 28h 11(5,3,3)
        break;
      case 1053:
        regSP--; break;
      case 1054:
        MEM_WRITE(regSP, regPCh); break;
      case 1055:
        regSP--; break;
      case 1056:
        break;
      case 1057:
        MEM_WRITE(regSP, regPCl); break;
      case 1058:
        break;
      case 1059:
        regPC = 0x28; OP_FETCH; break;
      case 1060: // ret p 5(5) 11(5,3,3)
        break;
      case 1061:
        if (FFLAG(SF)) { OP_FETCH; } break;
      case 1062:
        MEM_READ(regSP); break;
      case 1063:
        regSP++; break;
      case 1064:
        regPCl = cpuData; break;
      case 1065:
        MEM_READ(regSP); break;
      case 1066:
        regSP++; break;
      case 1067:
        regPCh = cpuData; OP_FETCH; break;
      case 1068: // pop af 10(4,3,3)
        break;
      case 1069:
        MEM_READ(regSP); break;
      case 1070:
        regSP++; break;
      case 1071:
        regF = cpuData; break;
      case 1072:
        MEM_READ(regSP); break;
      case 1073:
        regSP++; break;
      case 1074:
        regA = cpuData; OP_FETCH; break;
      case 1075: // jp p,NN 10(4,3,3)
        break;
      case 1076:
        MEM_READ(regPC++); break;
      case 1077:
        break;
      case 1078:
        regZ = cpuData; break;
      case 1079:
        MEM_READ(regPC++); break;
      case 1080:
        break;
      case 1081:
        regW = cpuData; if (!FFLAG(SF)) { regPC = regWZ; } OP_FETCH; break;
      case 1082: // di 4(4)
        intFF1 = intFF2 = 0; OP_FETCH; break;
      case 1083: // call p,NN 10(4,3,3) 17(4,3,4,3,3)
        break;
      case 1084:
        MEM_READ(regPC++); break;
      case 1085:
        break;
      case 1086:
        regZ = cpuData; break;
      case 1087:
        MEM_READ(regPC++); break;
      case 1088:
        break;
      case 1089:
        regW = cpuData; if (FFLAG(SF)) { OP_FETCH; } break;
      case 1090:
        regSP--; break;
      case 1091:
        MEM_WRITE(regSP, regPCh); break;
      case 1092:
        regSP--; break;
      case 1093:
        break;
      case 1094:
        MEM_WRITE(regSP, regPCl); break;
      case 1095:
        break;
      case 1096:
        regPC = regWZ; OP_FETCH; break;
      case 1097: // push af 11(5,3,3)
        regWZ = regAF; break;
      case 1098:
        regSP--; break;
      case 1099:
        MEM_WRITE(regSP, regW); break;
      case 1100:
        regSP--; break;
      case 1101:
        break;
      case 1102:
        MEM_WRITE(regSP, regZ); break;
      case 1103:
        break;
      case 1104:
        OP_FETCH; break;
      case 1105: // or N 7(4,3)
        break;
      case 1106:
        MEM_READ(regPC++); break;
      case 1107:
        break;
      case 1108:
        regV = cpuData; ALU_Or(); OP_FETCH; break;
      case 1109: // rst 30h 11(5,3,3)
        break;
      case 1110:
        regSP--; break;
      case 1111:
        MEM_WRITE(regSP, regPCh); break;
      case 1112:
        regSP--; break;
      case 1113:
        break;
      case 1114:
        MEM_WRITE(regSP, regPCl); break;
      case 1115:
        break;
      case 1116:
        regPC = 0x30; OP_FETCH; break;
      case 1117: // ret m 5(5) 11(5,3,3)
        break;
      case 1118:
        if (!FFLAG(SF)) { OP_FETCH; } break;
      case 1119:
        MEM_READ(regSP); break;
      case 1120:
        regSP++; break;
      case 1121:
        regPCl = cpuData; break;
      case 1122:
        MEM_READ(regSP); break;
      case 1123:
        regSP++; break;
      case 1124:
        regPCh = cpuData; OP_FETCH; break;
      case 1125: // ld sp,hl 6(6) ld sp,ix/iy 10(4,6)
        break;
      case 1126:
        regSP = regHLx; break;
      case 1127:
        OP_FETCH; break;
      case 1128: // jp m,NN 10(4,3,3)
        break;
      case 1129:
        MEM_READ(regPC++); break;
      case 1130:
        break;
      case 1131:
        regZ = cpuData; break;
      case 1132:
        MEM_READ(regPC++); break;
      case 1133:
        break;
      case 1134:
        regW = cpuData; if (FFLAG(SF)) { regPC = regWZ; } OP_FETCH; break;
      case 1135: // ei 4(4)
        OP_FETCH; intFF1 = intFF2 = 1; break;
      case 1136: // call m,NN 10(4,3,3) 17(4,3,4,3,3)
        break;
      case 1137:
        MEM_READ(regPC++); break;
      case 1138:
        break;
      case 1139:
        regZ = cpuData; break;
      case 1140:
        MEM_READ(regPC++); break;
      case 1141:
        break;
      case 1142:
        regW = cpuData; if (!FFLAG(SF)) { OP_FETCH; } break;
      case 1143:
        regSP--; break;
      case 1144:
        MEM_WRITE(regSP, regPCh); break;
      case 1145:
        regSP--; break;
      case 1146:
        break;
      case 1147:
        MEM_WRITE(regSP, regPCl); break;
      case 1148:
        break;
      case 1149:
        regPC = regWZ; OP_FETCH; break;
      case 1150: // FD
        USE_IY; M1_FETCH(0); break;
      case 1151: // cp N 7(4,3)
        break;
      case 1152:
        MEM_READ(regPC++); break;
      case 1153:
        break;
      case 1154:
        regV = cpuData; ALU_Cp(); OP_FETCH; break;
      case 1155: // rst 38h 11(5,3,3)
        break;
      case 1156:
        regSP--; break;
      case 1157:
        MEM_WRITE(regSP, regPCh); break;
      case 1158:
        regSP--; break;
      case 1159:
        break;
      case 1160:
        MEM_WRITE(regSP, regPCl); break;
      case 1161:
        break;
      case 1162:
        regPC = 0x38; OP_FETCH; break;
      case 1163: // in b,(c) 12(4,4,4)
        break;
      case 1164:
        BUS_ADD(regBC); break;
      case 1165:
        IO_READ; break;
      case 1166:
        break;
      case 1167:
        regB = cpuData; regF = IS_SZP(regB) | FFLAG(CF); OP_FETCH; break;
      case 1168: // out (c),b 12(4,4,4)
        break;
      case 1169:
        BUS_ADD_DATA(regBC, regB); break;
      case 1170:
        IO_WRITE; break;
      case 1171:
        break;
      case 1172:
        OP_FETCH; break;
      case 1173: // sbc hl,bc 15(4,4,4,3)
        break;
      case 1174:
        regQ = ZF; ALU_Sub(regL, regC, FFLAG(CF)); break;
      case 1175:
        regL = regV; break;
      case 1176:
        break;
      case 1177:
        break;
      case 1178:
        ALU_Sub(regH, regB, QFLAG(CF)); break;
      case 1179:
        regH = regV; break;
      case 1180:
        regF = regQ; OP_FETCH; break;
      case 1181: // ld (NN),bc 20(4,4,3,3,3,3)
        break;
      case 1182:
        MEM_READ(regPC++); break;
      case 1183:
        break;
      case 1184:
        regZ = cpuData; break;
      case 1185:
        MEM_READ(regPC++); break;
      case 1186:
        break;
      case 1187:
        regW = cpuData; break;
      case 1188:
        MEM_WRITE(regWZ, regC); break;
      case 1189:
        regWZ++; break;
      case 1190:
        break;
      case 1191:
        MEM_WRITE(regWZ, regB); break;
      case 1192:
        break;
      case 1193:
        OP_FETCH; break;
      case 1194: // neg 8(4,4)
        regQ = ZF; ALU_Sub(0, regA, 0); regAF = regVQ; OP_FETCH; break;
      case 1195: // retn 14(4,4,3,3)
        break;
      case 1196:
        MEM_READ(regSP); break;
      case 1197:
        regSP++; break;
      case 1198:
        regPCl = cpuData; break;
      case 1199:
        MEM_READ(regSP); break;
      case 1200:
        regSP++; break;
      case 1201:
        regPCh = cpuData; intFF1 = intFF2; OP_FETCH; break;
      case 1202: // im 0 8(4,4)
        intMode = 0; OP_FETCH; break;
      case 1203: // ld i,a 9(4,5)
        regI = regA; break;
      case 1204:
        OP_FETCH; break;
      case 1205: // in c,(c) 12(4,4,4)
        break;
      case 1206:
        BUS_ADD(regBC); break;
      case 1207:
        IO_READ; break;
      case 1208:
        break;
      case 1209:
        regC = cpuData; regF = IS_SZP(regC) | FFLAG(CF); OP_FETCH; break;
      case 1210: // out (c),c 12(4,4,4)
        break;
      case 1211:
        BUS_ADD_DATA(regBC, regC); break;
      case 1212:
        IO_WRITE; break;
      case 1213:
        break;
      case 1214:
        OP_FETCH; break;
      case 1215: // adc hl,bc 15(4,4,4,3)
        break;
      case 1216:
        regQ = ZF; ALU_Add(regL, regC, FFLAG(CF)); break;
      case 1217:
        regL = regV; break;
      case 1218:
        break;
      case 1219:
        break;
      case 1220:
        ALU_Add(regH, regB, QFLAG(CF)); break;
      case 1221:
        regH = regV; break;
      case 1222:
        regF = regQ; OP_FETCH; break;
      case 1223: // ld bc,(NN) 20(4,4,3,3,3,3)
        break;
      case 1224:
        MEM_READ(regPC++); break;
      case 1225:
        break;
      case 1226:
        regZ = cpuData; break;
      case 1227:
        MEM_READ(regPC++); break;
      case 1228:
        break;
      case 1229:
        regW = cpuData; break;
      case 1230:
        MEM_READ(regWZ); break;
      case 1231:
        regWZ++; break;
      case 1232:
        regC = cpuData; break;
      case 1233:
        MEM_READ(regWZ); break;
      case 1234:
        break;
      case 1235:
        regB = cpuData; OP_FETCH; break;
      case 1236: // reti 14(4,4,3,3)
        break;
      case 1237:
        MEM_READ(regSP); break;
      case 1238:
        regSP++; break;
      case 1239:
        regPCl = cpuData; break;
      case 1240:
        MEM_READ(regSP); break;
      case 1241:
        regSP++; break;
      case 1242:
        regPCh = cpuData; intFF1 = intFF2; OP_FETCH; break;
      case 1243: // ld r,a 9(4,5)
        regR = regA; break;
      case 1244:
        OP_FETCH; break;
      case 1245: // in d,(c) 12(4,4,4)
        break;
      case 1246:
        BUS_ADD(regBC); break;
      case 1247:
        IO_READ; break;
      case 1248:
        break;
      case 1249:
        regD = cpuData; regF = IS_SZP(regD) | FFLAG(CF); OP_FETCH; break;
      case 1250: // out (c),d 12(4,4,4)
        break;
      case 1251:
        BUS_ADD_DATA(regBC, regD); break;
      case 1252:
        IO_WRITE; break;
      case 1253:
        break;
      case 1254:
        OP_FETCH; break;
      case 1255: // sbc hl,de 15(4,4,4,3)
        break;
      case 1256:
        regQ = ZF; ALU_Sub(regL, regE, FFLAG(CF)); break;
      case 1257:
        regL = regV; break;
      case 1258:
        break;
      case 1259:
        break;
      case 1260:
        ALU_Sub(regH, regD, QFLAG(CF)); break;
      case 1261:
        regH = regV; break;
      case 1262:
        regF = regQ; OP_FETCH; break;
      case 1263: // ld (NN),de 20(4,4,3,3,3,3)
        break;
      case 1264:
        MEM_READ(regPC++); break;
      case 1265:
        break;
      case 1266:
        regZ = cpuData; break;
      case 1267:
        MEM_READ(regPC++); break;
      case 1268:
        break;
      case 1269:
        regW = cpuData; break;
      case 1270:
        MEM_WRITE(regWZ, regE); break;
      case 1271:
        regWZ++; break;
      case 1272:
        break;
      case 1273:
        MEM_WRITE(regWZ, regD); break;
      case 1274:
        break;
      case 1275:
        OP_FETCH; break;
      case 1276: // im 1 8(4,4)
        intMode = 1; OP_FETCH; break;
      case 1277: // ld a,i 9(4,5)
        regV = regI; regQ = (IS_SZP(regV) & (~PF)) | IFF2_PF | FFLAG(CF); break;
      case 1278:
        regAF = regVQ; OP_FETCH; break;
      case 1279: // in e,(c) 12(4,4,4)
        break;
      case 1280:
        BUS_ADD(regBC); break;
      case 1281:
        IO_READ; break;
      case 1282:
        break;
      case 1283:
        regE = cpuData; regF = IS_SZP(regE) | FFLAG(CF); OP_FETCH; break;
      case 1284: // out (c),e 12(4,4,4)
        break;
      case 1285:
        BUS_ADD_DATA(regBC, regE); break;
      case 1286:
        IO_WRITE; break;
      case 1287:
        break;
      case 1288:
        OP_FETCH; break;
      case 1289: // adc hl,de 15(4,4,4,3)
        break;
      case 1290:
        regQ = ZF; ALU_Add(regL, regE, FFLAG(CF)); break;
      case 1291:
        regL = regV; break;
      case 1292:
        break;
      case 1293:
        break;
      case 1294:
        ALU_Add(regH, regD, QFLAG(CF)); break;
      case 1295:
        regH = regV; break;
      case 1296:
        regF = regQ; OP_FETCH; break;
      case 1297: // ld de,(NN) 20(4,4,3,3,3,3)
        break;
      case 1298:
        MEM_READ(regPC++); break;
      case 1299:
        break;
      case 1300:
        regZ = cpuData; break;
      case 1301:
        MEM_READ(regPC++); break;
      case 1302:
        break;
      case 1303:
        regW = cpuData; break;
      case 1304:
        MEM_READ(regWZ); break;
      case 1305:
        regWZ++; break;
      case 1306:
        regE = cpuData; break;
      case 1307:
        MEM_READ(regWZ); break;
      case 1308:
        break;
      case 1309:
        regD = cpuData; OP_FETCH; break;
      case 1310: // im 2 8(4,4)
        intMode = 2; OP_FETCH; break;
      case 1311: // ld a,r 9(4,5)
        regV = regR; regQ = (IS_SZP(regV) & (~PF)) | IFF2_PF | FFLAG(CF); break;
      case 1312:
        regAF = regVQ; OP_FETCH; break;
      case 1313: // in h,(c) 12(4,4,4)
        break;
      case 1314:
        BUS_ADD(regBC); break;
      case 1315:
        IO_READ; break;
      case 1316:
        break;
      case 1317:
        regH = cpuData; regF = IS_SZP(regH) | FFLAG(CF); OP_FETCH; break;
      case 1318: // out (c),h 12(4,4,4)
        break;
      case 1319:
        BUS_ADD_DATA(regBC, regH); break;
      case 1320:
        IO_WRITE; break;
      case 1321:
        break;
      case 1322:
        OP_FETCH; break;
      case 1323: // sbc hl,hl 15(4,4,4,3)
        break;
      case 1324:
        regQ = ZF; ALU_Sub(regL, regL, FFLAG(CF)); break;
      case 1325:
        regL = regV; break;
      case 1326:
        break;
      case 1327:
        break;
      case 1328:
        ALU_Sub(regH, regH, QFLAG(CF)); break;
      case 1329:
        regH = regV; break;
      case 1330:
        regF = regQ; OP_FETCH; break;
      case 1331: // rrd 18(4,4,3,4,3)
        break;
      case 1332:
        MEM_READ(regHL); break;
      case 1333:
        break;
      case 1334:
        regV = cpuData; break;
      case 1335:
        break;
      case 1336:
        break;
      case 1337:
        break;
      case 1338:
        break;
      case 1339:
        MEM_WRITE(regHL, (regA << 4) | (regV >> 4)); break;
      case 1340:
        break;
      case 1341:
        regA = (regA & 0xf0) | (regV & 0x0f); regF = IS_SZP(regA) | FFLAG(CF); OP_FETCH; break;
      case 1342: // in l,(c) 12(4,4,4)
        break;
      case 1343:
        BUS_ADD(regBC); break;
      case 1344:
        IO_READ; break;
      case 1345:
        break;
      case 1346:
        regL = cpuData; regF = IS_SZP(regL) | FFLAG(CF); OP_FETCH; break;
      case 1347: // out (c),l 12(4,4,4)
        break;
      case 1348:
        BUS_ADD_DATA(regBC, regL); break;
      case 1349:
        IO_WRITE; break;
      case 1350:
        break;
      case 1351:
        OP_FETCH; break;
      case 1352: // adc hl,hl 15(4,4,4,3)
        break;
      case 1353:
        regQ = ZF; ALU_Add(regL, regL, FFLAG(CF)); break;
      case 1354:
        regL = regV; break;
      case 1355:
        break;
      case 1356:
        break;
      case 1357:
        ALU_Add(regH, regH, QFLAG(CF)); break;
      case 1358:
        regH = regV; break;
      case 1359:
        regF = regQ; OP_FETCH; break;
      case 1360: // rld 18(4,4,3,4,3)
        break;
      case 1361:
        MEM_READ(regHL); break;
      case 1362:
        break;
      case 1363:
        regV = cpuData; break;
      case 1364:
        break;
      case 1365:
        break;
      case 1366:
        break;
      case 1367:
        break;
      case 1368:
        MEM_WRITE(regHL, (regV << 4) | (regA & 0x0f)); break;
      case 1369:
        break;
      case 1370:
        regA = (regA & 0xf0) | (regV >> 4); regF = IS_SZP(regA) | FFLAG(CF); OP_FETCH; break;
      case 1371: // in (c) 12(4,4,4)
        break;
      case 1372:
        BUS_ADD(regBC); break;
      case 1373:
        IO_READ; break;
      case 1374:
        break;
      case 1375:
        regF = IS_SZP(cpuData) | FFLAG(CF); OP_FETCH; break;
      case 1376: // out (c),0 12(4,4,4)
        break;
      case 1377:
        BUS_ADD_DATA(regBC, 0); break;
      case 1378:
        IO_WRITE; break;
      case 1379:
        break;
      case 1380:
        OP_FETCH; break;
      case 1381: // sbc hl,sp 15(4,4,4,3)
        break;
      case 1382:
        regQ = ZF; ALU_Sub(regL, regSPl, FFLAG(CF)); break;
      case 1383:
        regL = regV; break;
      case 1384:
        break;
      case 1385:
        break;
      case 1386:
        ALU_Sub(regH, regSPh, QFLAG(CF)); break;
      case 1387:
        regH = regV; break;
      case 1388:
        regF = regQ; OP_FETCH; break;
      case 1389: // ld (NN),sp 20(4,4,3,3,3,3)
        break;
      case 1390:
        MEM_READ(regPC++); break;
      case 1391:
        break;
      case 1392:
        regZ = cpuData; break;
      case 1393:
        MEM_READ(regPC++); break;
      case 1394:
        break;
      case 1395:
        regW = cpuData; break;
      case 1396:
        MEM_WRITE(regWZ, regSPl); break;
      case 1397:
        regWZ++; break;
      case 1398:
        break;
      case 1399:
        MEM_WRITE(regWZ, regSPh); break;
      case 1400:
        break;
      case 1401:
        OP_FETCH; break;
      case 1402: // in a,(c) 12(4,4,4)
        break;
      case 1403:
        BUS_ADD(regBC); break;
      case 1404:
        IO_READ; break;
      case 1405:
        break;
      case 1406:
        regA = cpuData; regF = IS_SZP(regA) | FFLAG(CF); OP_FETCH; break;
      case 1407: // out (c),a 12(4,4,4)
        break;
      case 1408:
        BUS_ADD_DATA(regBC, regA); break;
      case 1409:
        IO_WRITE; break;
      case 1410:
        break;
      case 1411:
        OP_FETCH; break;
      case 1412: // adc hl,sp 15(4,4,4,3)
        break;
      case 1413:
        regQ = ZF; ALU_Add(regL, regSPl, FFLAG(CF)); break;
      case 1414:
        regL = regV; break;
      case 1415:
        break;
      case 1416:
        break;
      case 1417:
        ALU_Add(regH, regSPh, QFLAG(CF)); break;
      case 1418:
        regH = regV; break;
      case 1419:
        regF = regQ; OP_FETCH; break;
      case 1420: // ld sp,(NN) 20(4,4,3,3,3,3)
        break;
      case 1421:
        MEM_READ(regPC++); break;
      case 1422:
        break;
      case 1423:
        regZ = cpuData; break;
      case 1424:
        MEM_READ(regPC++); break;
      case 1425:
        break;
      case 1426:
        regW = cpuData; break;
      case 1427:
        MEM_READ(regWZ); break;
      case 1428:
        regWZ++; break;
      case 1429:
        regSPl = cpuData; break;
      case 1430:
        MEM_READ(regWZ); break;
      case 1431:
        break;
      case 1432:
        regSPh = cpuData; OP_FETCH; break;
      case 1433: // ldi 16(4,4,3,5)
        break;
      case 1434:
        MEM_READ(regHL); break;
      case 1435:
        regHL++; break;
      case 1436:
        regV = cpuData; break;
      case 1437:
        MEM_WRITE(regDE, regV); break;
      case 1438:
        regDE++; break;
      case 1439:
        regQ = FFLAG(SF | ZF | CF); break;
      case 1440:
        regBC--; SET_PF(regB | regC); break;
      case 1441:
        regF = regQ; OP_FETCH; break;
      case 1442: // cpi 16(4,4,3,5)
        break;
      case 1443:
        MEM_READ(regHL); break;
      case 1444:
        regHL++; break;
      case 1445:
        regV = cpuData; break;
      case 1446:
        regQ = ZF; ALU_Sub(regA, regV, 0); break;
      case 1447:
        regQ = QFLAG(~(CF | PF)) | FFLAG(CF); break;
      case 1448:
        break;
      case 1449:
        regBC--; SET_PF(regB | regC); break;
      case 1450:
        regF = regQ; OP_FETCH; break;
      case 1451: // ini 16(4,5,4,3)
        break;
      case 1452:
        break;
      case 1453:
        BUS_ADD(regBC); break;
      case 1454:
        IO_READ; regB--; break;
      case 1455:
        break;
      case 1456:
        regV = cpuData; break;
      case 1457:
        MEM_WRITE(regHL, regV); break;
      case 1458:
        regHL++; regQ = FFLAG(~ZF) | IS_ZF(regB) | NF; break;
      case 1459:
        regF = regQ; OP_FETCH; break;
      case 1460: // outi 16(4,5,3,4)
        break;
      case 1461:
        break;
      case 1462:
        MEM_READ(regHL); regB--; break;
      case 1463:
        regHL++; break;
      case 1464:
        regV = cpuData; break;
      case 1465:
        BUS_ADD_DATA(regBC, regV); break;
      case 1466:
        IO_WRITE; break;
      case 1467:
        regQ = FFLAG(~ZF) | IS_ZF(regB) | NF; break;
      case 1468:
        regF = regQ; OP_FETCH; break;
      case 1469: // ldd 16(4,4,3,5)
        break;
      case 1470:
        MEM_READ(regHL); break;
      case 1471:
        regHL--; break;
      case 1472:
        regV = cpuData; break;
      case 1473:
        MEM_WRITE(regDE, regV); break;
      case 1474:
        regDE--; break;
      case 1475:
        regQ = FFLAG(SF | ZF | CF); break;
      case 1476:
        regBC--; SET_PF(regB | regC); break;
      case 1477:
        regF = regQ; OP_FETCH; break;
      case 1478: // cpd 16(4,4,3,5)
        break;
      case 1479:
        MEM_READ(regHL); break;
      case 1480:
        regHL--; break;
      case 1481:
        regV = cpuData; break;
      case 1482:
        regQ = ZF; ALU_Sub(regA, regV, 0); break;
      case 1483:
        regQ = QFLAG(~(CF | PF)) | FFLAG(CF); break;
      case 1484:
        break;
      case 1485:
        regBC--; SET_PF(regB | regC); break;
      case 1486:
        regF = regQ; OP_FETCH; break;
      case 1487: // ind 16(4,5,4,3)
        break;
      case 1488:
        break;
      case 1489:
        BUS_ADD(regBC); break;
      case 1490:
        IO_READ; regB--; break;
      case 1491:
        break;
      case 1492:
        regV = cpuData; break;
      case 1493:
        MEM_WRITE(regHL, regV); break;
      case 1494:
        regHL--; regQ = FFLAG(~ZF) | IS_ZF(regB) | NF; break;
      case 1495:
        regF = regQ; OP_FETCH; break;
      case 1496: // outd 16(4,5,3,4)
        break;
      case 1497:
        break;
      case 1498:
        MEM_READ(regHL); regB--; break;
      case 1499:
        regHL--; break;
      case 1500:
        regV = cpuData; break;
      case 1501:
        BUS_ADD_DATA(regBC, regV); break;
      case 1502:
        IO_WRITE; break;
      case 1503:
        regQ = FFLAG(~ZF) | IS_ZF(regB) | NF; break;
      case 1504:
        regF = regQ; OP_FETCH; break;
      case 1505: // ldir 16(4,4,3,5) 21(4,4,3,5,5)
        break;
      case 1506:
        MEM_READ(regHL); break;
      case 1507:
        regHL++; break;
      case 1508:
        regV = cpuData; break;
      case 1509:
        MEM_WRITE(regDE, regV); break;
      case 1510:
        regDE++; break;
      case 1511:
        regQ = FFLAG(SF | ZF | CF); break;
      case 1512:
        regBC--; SET_PF(regB | regC); break;
      case 1513:
        regF = regQ; if (!FFLAG(PF)) { OP_FETCH; } break;
      case 1514:
        break;
      case 1515:
        break;
      case 1516:
        break;
      case 1517:
        regPC -= 2; break;
      case 1518:
        OP_FETCH; break;
      case 1519: // cpir 16(4,4,3,5) 21(4,4,3,5,5)
        break;
      case 1520:
        MEM_READ(regHL); break;
      case 1521:
        regHL++; break;
      case 1522:
        regV = cpuData; break;
      case 1523:
        regQ = ZF; ALU_Sub(regA, regV, 0); break;
      case 1524:
        regQ = QFLAG(~(CF | PF)) | FFLAG(CF); break;
      case 1525:
        break;
      case 1526:
        regBC--; SET_PF(regB | regC); break;
      case 1527:
        regF = regQ; if (FFLAG(ZF) || (!FFLAG(PF))) { OP_FETCH; } break;
      case 1528:
        break;
      case 1529:
        break;
      case 1530:
        break;
      case 1531:
        regPC -= 2; break;
      case 1532:
        OP_FETCH; break;
      case 1533: // inir 16(4,5,4,3) 21(4,5,4,3,5)
        break;
      case 1534:
        break;
      case 1535:
        BUS_ADD(regBC); break;
      case 1536:
        IO_READ; regB--; break;
      case 1537:
        break;
      case 1538:
        regV = cpuData; break;
      case 1539:
        MEM_WRITE(regHL, regV); break;
      case 1540:
        regHL++; regQ = FFLAG(~ZF) | IS_ZF(regB) | NF; break;
      case 1541:
        regF = regQ; if (FFLAG(ZF)) { OP_FETCH; } break;
      case 1542:
        break;
      case 1543:
        break;
      case 1544:
        break;
      case 1545:
        regPC -= 2; break;
      case 1546:
        OP_FETCH; break;
      case 1547: // otir 16(4,5,3,4) 21(4,5,3,4,5)
        break;
      case 1548:
        break;
      case 1549:
        MEM_READ(regHL); regB--; break;
      case 1550:
        regHL++; break;
      case 1551:
        regV = cpuData; break;
      case 1552:
        BUS_ADD_DATA(regBC, regV); break;
      case 1553:
        IO_WRITE; break;
      case 1554:
        regQ = FFLAG(~ZF) | IS_ZF(regB) | NF; break;
      case 1555:
        regF = regQ; if (FFLAG(ZF)) { OP_FETCH; } break;
      case 1556:
        break;
      case 1557:
        break;
      case 1558:
        break;
      case 1559:
        regPC -= 2; break;
      case 1560:
        OP_FETCH; break;
      case 1561: // lddr 16(4,4,3,5) 21(4,4,3,5,5)
        break;
      case 1562:
        MEM_READ(regHL); break;
      case 1563:
        regHL--; break;
      case 1564:
        regV = cpuData; break;
      case 1565:
        MEM_WRITE(regDE, regV); break;
      case 1566:
        regDE--; break;
      case 1567:
        regQ = FFLAG(SF | ZF | CF); break;
      case 1568:
        regBC--; SET_PF(regB | regC); break;
      case 1569:
        regF = regQ; if (!FFLAG(PF)) { OP_FETCH; } break;
      case 1570:
        break;
      case 1571:
        break;
      case 1572:
        break;
      case 1573:
        regPC -= 2; break;
      case 1574:
        OP_FETCH; break;
      case 1575: // cpdr 16(4,4,3,5) 21(4,4,3,5,5)
        break;
      case 1576:
        MEM_READ(regHL); break;
      case 1577:
        regHL--; break;
      case 1578:
        regV = cpuData; break;
      case 1579:
        regQ = ZF; ALU_Sub(regA, regV, 0); break;
      case 1580:
        regQ = QFLAG(~(CF | PF)) | FFLAG(CF); break;
      case 1581:
        break;
      case 1582:
        regBC--; SET_PF(regB | regC); break;
      case 1583:
        regF = regQ; if (FFLAG(ZF) || (!FFLAG(PF))) { OP_FETCH; } break;
      case 1584:
        break;
      case 1585:
        break;
      case 1586:
        break;
      case 1587:
        regPC -= 2; break;
      case 1588:
        OP_FETCH; break;
      case 1589: // indr 16(4,5,4,3) 21(4,5,4,3,5)
        break;
      case 1590:
        break;
      case 1591:
        BUS_ADD(regBC); break;
      case 1592:
        IO_READ; regB--; break;
      case 1593:
        break;
      case 1594:
        regV = cpuData; break;
      case 1595:
        MEM_WRITE(regHL, regV); break;
      case 1596:
        regHL--; regQ = FFLAG(~ZF) | IS_ZF(regB) | NF; break;
      case 1597:
        regF = regQ; if (FFLAG(ZF)) { OP_FETCH; } break;
      case 1598:
        break;
      case 1599:
        break;
      case 1600:
        break;
      case 1601:
        regPC -= 2; break;
      case 1602:
        OP_FETCH; break;
      case 1603: // otdr 16(4,5,3,4) 21(4,5,3,4,5)
        break;
      case 1604:
        break;
      case 1605:
        MEM_READ(regHL); regB--; break;
      case 1606:
        regHL--; break;
      case 1607:
        regV = cpuData; break;
      case 1608:
        BUS_ADD_DATA(regBC, regV); break;
      case 1609:
        IO_WRITE; break;
      case 1610:
        regQ = FFLAG(~ZF) | IS_ZF(regB) | NF; break;
      case 1611:
        regF = regQ; if (FFLAG(ZF)) { OP_FETCH; } break;
      case 1612:
        break;
      case 1613:
        break;
      case 1614:
        break;
      case 1615:
        regPC -= 2; break;
      case 1616:
        OP_FETCH; break;
      case 1617: // rlc b 8(4,4)
        regV = regB; ALU_RLC(); regB = regV; regF = regQ; OP_FETCH; break;
      case 1618: // rlc c 8(4,4)
        regV = regC; ALU_RLC(); regC = regV; regF = regQ; OP_FETCH; break;
      case 1619: // rlc d 8(4,4)
        regV = regD; ALU_RLC(); regD = regV; regF = regQ; OP_FETCH; break;
      case 1620: // rlc e 8(4,4)
        regV = regE; ALU_RLC(); regE = regV; regF = regQ; OP_FETCH; break;
      case 1621: // rlc h 8(4,4)
        regV = regH; ALU_RLC(); regH = regV; regF = regQ; OP_FETCH; break;
      case 1622: // rlc l 8(4,4)
        regV = regL; ALU_RLC(); regL = regV; regF = regQ; OP_FETCH; break;
      case 1623: // rlc (hl) 15(4,4,4,3)
        break;
      case 1624:
        MEM_READ(regHL); break;
      case 1625:
        break;
      case 1626:
        regV = cpuData; break;
      case 1627:
        ALU_RLC(); break;
      case 1628:
        MEM_WRITE(regHL, regV); break;
      case 1629:
        break;
      case 1630:
        regF = regQ; OP_FETCH; break;
      case 1631: // rlc a 8(4,4)
        regV = regA; ALU_RLC(); regA = regV; regF = regQ; OP_FETCH; break;
      case 1632: // rrc b 8(4,4)
        regV = regB; ALU_RRC(); regB = regV; regF = regQ; OP_FETCH; break;
      case 1633: // rrc c 8(4,4)
        regV = regC; ALU_RRC(); regC = regV; regF = regQ; OP_FETCH; break;
      case 1634: // rrc d 8(4,4)
        regV = regD; ALU_RRC(); regD = regV; regF = regQ; OP_FETCH; break;
      case 1635: // rrc e 8(4,4)
        regV = regE; ALU_RRC(); regE = regV; regF = regQ; OP_FETCH; break;
      case 1636: // rrc h 8(4,4)
        regV = regH; ALU_RRC(); regH = regV; regF = regQ; OP_FETCH; break;
      case 1637: // rrc l 8(4,4)
        regV = regL; ALU_RRC(); regL = regV; regF = regQ; OP_FETCH; break;
      case 1638: // rrc (hl) 15(4,4,4,3)
        break;
      case 1639:
        MEM_READ(regHL); break;
      case 1640:
        break;
      case 1641:
        regV = cpuData; break;
      case 1642:
        ALU_RRC(); break;
      case 1643:
        MEM_WRITE(regHL, regV); break;
      case 1644:
        break;
      case 1645:
        regF = regQ; OP_FETCH; break;
      case 1646: // rrc a 8(4,4)
        regV = regA; ALU_RRC(); regA = regV; regF = regQ; OP_FETCH; break;
      case 1647: // rl b 8(4,4)
        regV = regB; ALU_RL(); regB = regV; regF = regQ; OP_FETCH; break;
      case 1648: // rl c 8(4,4)
        regV = regC; ALU_RL(); regC = regV; regF = regQ; OP_FETCH; break;
      case 1649: // rl d 8(4,4)
        regV = regD; ALU_RL(); regD = regV; regF = regQ; OP_FETCH; break;
      case 1650: // rl e 8(4,4)
        regV = regE; ALU_RL(); regE = regV; regF = regQ; OP_FETCH; break;
      case 1651: // rl h 8(4,4)
        regV = regH; ALU_RL(); regH = regV; regF = regQ; OP_FETCH; break;
      case 1652: // rl l 8(4,4)
        regV = regL; ALU_RL(); regL = regV; regF = regQ; OP_FETCH; break;
      case 1653: // rl (hl) 15(4,4,4,3)
        break;
      case 1654:
        MEM_READ(regHL); break;
      case 1655:
        break;
      case 1656:
        regV = cpuData; break;
      case 1657:
        ALU_RL(); break;
      case 1658:
        MEM_WRITE(regHL, regV); break;
      case 1659:
        break;
      case 1660:
        regF = regQ; OP_FETCH; break;
      case 1661: // rl a 8(4,4)
        regV = regA; ALU_RL(); regA = regV; regF = regQ; OP_FETCH; break;
      case 1662: // rr b 8(4,4)
        regV = regB; ALU_RR(); regB = regV; regF = regQ; OP_FETCH; break;
      case 1663: // rr c 8(4,4)
        regV = regC; ALU_RR(); regC = regV; regF = regQ; OP_FETCH; break;
      case 1664: // rr d 8(4,4)
        regV = regD; ALU_RR(); regD = regV; regF = regQ; OP_FETCH; break;
      case 1665: // rr e 8(4,4)
        regV = regE; ALU_RR(); regE = regV; regF = regQ; OP_FETCH; break;
      case 1666: // rr h 8(4,4)
        regV = regH; ALU_RR(); regH = regV; regF = regQ; OP_FETCH; break;
      case 1667: // rr l 8(4,4)
        regV = regL; ALU_RR(); regL = regV; regF = regQ; OP_FETCH; break;
      case 1668: // rr (hl) 15(4,4,4,3)
        break;
      case 1669:
        MEM_READ(regHL); break;
      case 1670:
        break;
      case 1671:
        regV = cpuData; break;
      case 1672:
        ALU_RR(); break;
      case 1673:
        MEM_WRITE(regHL, regV); break;
      case 1674:
        break;
      case 1675:
        regF = regQ; OP_FETCH; break;
      case 1676: // rr a 8(4,4)
        regV = regA; ALU_RR(); regA = regV; regF = regQ; OP_FETCH; break;
      case 1677: // sla b 8(4,4)
        regV = regB; ALU_SLA(); regB = regV; regF = regQ; OP_FETCH; break;
      case 1678: // sla c 8(4,4)
        regV = regC; ALU_SLA(); regC = regV; regF = regQ; OP_FETCH; break;
      case 1679: // sla d 8(4,4)
        regV = regD; ALU_SLA(); regD = regV; regF = regQ; OP_FETCH; break;
      case 1680: // sla e 8(4,4)
        regV = regE; ALU_SLA(); regE = regV; regF = regQ; OP_FETCH; break;
      case 1681: // sla h 8(4,4)
        regV = regH; ALU_SLA(); regH = regV; regF = regQ; OP_FETCH; break;
      case 1682: // sla l 8(4,4)
        regV = regL; ALU_SLA(); regL = regV; regF = regQ; OP_FETCH; break;
      case 1683: // sla (hl) 15(4,4,4,3)
        break;
      case 1684:
        MEM_READ(regHL); break;
      case 1685:
        break;
      case 1686:
        regV = cpuData; break;
      case 1687:
        ALU_SLA(); break;
      case 1688:
        MEM_WRITE(regHL, regV); break;
      case 1689:
        break;
      case 1690:
        regF = regQ; OP_FETCH; break;
      case 1691: // sla a 8(4,4)
        regV = regA; ALU_SLA(); regA = regV; regF = regQ; OP_FETCH; break;
      case 1692: // sra b 8(4,4)
        regV = regB; ALU_SRA(); regB = regV; regF = regQ; OP_FETCH; break;
      case 1693: // sra c 8(4,4)
        regV = regC; ALU_SRA(); regC = regV; regF = regQ; OP_FETCH; break;
      case 1694: // sra d 8(4,4)
        regV = regD; ALU_SRA(); regD = regV; regF = regQ; OP_FETCH; break;
      case 1695: // sra e 8(4,4)
        regV = regE; ALU_SRA(); regE = regV; regF = regQ; OP_FETCH; break;
      case 1696: // sra h 8(4,4)
        regV = regH; ALU_SRA(); regH = regV; regF = regQ; OP_FETCH; break;
      case 1697: // sra l 8(4,4)
        regV = regL; ALU_SRA(); regL = regV; regF = regQ; OP_FETCH; break;
      case 1698: // sra (hl) 15(4,4,4,3)
        break;
      case 1699:
        MEM_READ(regHL); break;
      case 1700:
        break;
      case 1701:
        regV = cpuData; break;
      case 1702:
        ALU_SRA(); break;
      case 1703:
        MEM_WRITE(regHL, regV); break;
      case 1704:
        break;
      case 1705:
        regF = regQ; OP_FETCH; break;
      case 1706: // sra a 8(4,4)
        regV = regA; ALU_SRA(); regA = regV; regF = regQ; OP_FETCH; break;
      case 1707: // sll b 8(4,4)
        regV = regB; ALU_SLL(); regB = regV; regF = regQ; OP_FETCH; break;
      case 1708: // sll c 8(4,4)
        regV = regC; ALU_SLL(); regC = regV; regF = regQ; OP_FETCH; break;
      case 1709: // sll d 8(4,4)
        regV = regD; ALU_SLL(); regD = regV; regF = regQ; OP_FETCH; break;
      case 1710: // sll e 8(4,4)
        regV = regE; ALU_SLL(); regE = regV; regF = regQ; OP_FETCH; break;
      case 1711: // sll h 8(4,4)
        regV = regH; ALU_SLL(); regH = regV; regF = regQ; OP_FETCH; break;
      case 1712: // sll l 8(4,4)
        regV = regL; ALU_SLL(); regL = regV; regF = regQ; OP_FETCH; break;
      case 1713: // sll (hl) 15(4,4,4,3)
        break;
      case 1714:
        MEM_READ(regHL); break;
      case 1715:
        break;
      case 1716:
        regV = cpuData; break;
      case 1717:
        ALU_SLL(); break;
      case 1718:
        MEM_WRITE(regHL, regV); break;
      case 1719:
        break;
      case 1720:
        regF = regQ; OP_FETCH; break;
      case 1721: // sll a 8(4,4)
        regV = regA; ALU_SLL(); regA = regV; regF = regQ; OP_FETCH; break;
      case 1722: // srl b 8(4,4)
        regV = regB; ALU_SRL(); regB = regV; regF = regQ; OP_FETCH; break;
      case 1723: // srl c 8(4,4)
        regV = regC; ALU_SRL(); regC = regV; regF = regQ; OP_FETCH; break;
      case 1724: // srl d 8(4,4)
        regV = regD; ALU_SRL(); regD = regV; regF = regQ; OP_FETCH; break;
      case 1725: // srl e 8(4,4)
        regV = regE; ALU_SRL(); regE = regV; regF = regQ; OP_FETCH; break;
      case 1726: // srl h 8(4,4)
        regV = regH; ALU_SRL(); regH = regV; regF = regQ; OP_FETCH; break;
      case 1727: // srl l 8(4,4)
        regV = regL; ALU_SRL(); regL = regV; regF = regQ; OP_FETCH; break;
      case 1728: // srl (hl) 15(4,4,4,3)
        break;
      case 1729:
        MEM_READ(regHL); break;
      case 1730:
        break;
      case 1731:
        regV = cpuData; break;
      case 1732:
        ALU_SRL(); break;
      case 1733:
        MEM_WRITE(regHL, regV); break;
      case 1734:
        break;
      case 1735:
        regF = regQ; OP_FETCH; break;
      case 1736: // srl a 8(4,4)
        regV = regA; ALU_SRL(); regA = regV; regF = regQ; OP_FETCH; break;
      case 1737: // bit 0,b 8(4,4)
        regV = regB; ALU_Bit(); OP_FETCH; break;
      case 1738: // bit 0,c 8(4,4)
        regV = regC; ALU_Bit(); OP_FETCH; break;
      case 1739: // bit 0,d 8(4,4)
        regV = regD; ALU_Bit(); OP_FETCH; break;
      case 1740: // bit 0,e 8(4,4)
        regV = regE; ALU_Bit(); OP_FETCH; break;
      case 1741: // bit 0,h 8(4,4)
        regV = regH; ALU_Bit(); OP_FETCH; break;
      case 1742: // bit 0,l 8(4,4)
        regV = regL; ALU_Bit(); OP_FETCH; break;
      case 1743: // bit 0,(hl) 12(4,4,4)
        break;
      case 1744:
        MEM_READ(regHL); break;
      case 1745:
        break;
      case 1746:
        regV = cpuData; break;
      case 1747:
        ALU_Bit(); OP_FETCH; break;
      case 1748: // bit 0,a 8(4,4)
        regV = regA; ALU_Bit(); OP_FETCH; break;
      case 1749: // res 0,b 8(4,4)
        regB &= ~BIT(IRh); OP_FETCH; break;
      case 1750: // res 0,c 8(4,4)
        regC &= ~BIT(IRh); OP_FETCH; break;
      case 1751: // res 0,d 8(4,4)
        regD &= ~BIT(IRh); OP_FETCH; break;
      case 1752: // res 0,e 8(4,4)
        regE &= ~BIT(IRh); OP_FETCH; break;
      case 1753: // res 0,h 8(4,4)
        regH &= ~BIT(IRh); OP_FETCH; break;
      case 1754: // res 0,l 8(4,4)
        regL &= ~BIT(IRh); OP_FETCH; break;
      case 1755: // res 0,(hl) 15(4,4,4,3)
        break;
      case 1756:
        MEM_READ(regHL); break;
      case 1757:
        break;
      case 1758:
        regV = cpuData; break;
      case 1759:
        regV &= ~BIT(IRh); break;
      case 1760:
        MEM_WRITE(regHL, regV); break;
      case 1761:
        break;
      case 1762:
        OP_FETCH; break;
      case 1763: // res 0,a 8(4,4)
        regA &= ~BIT(IRh); OP_FETCH; break;
      case 1764: // set 0,b 8(4,4)
        regB |= BIT(IRh); OP_FETCH; break;
      case 1765: // set 0,c 8(4,4)
        regC |= BIT(IRh); OP_FETCH; break;
      case 1766: // set 0,d 8(4,4)
        regD |= BIT(IRh); OP_FETCH; break;
      case 1767: // set 0,e 8(4,4)
        regE |= BIT(IRh); OP_FETCH; break;
      case 1768: // set 0,h 8(4,4)
        regH |= BIT(IRh); OP_FETCH; break;
      case 1769: // set 0,l 8(4,4)
        regL |= BIT(IRh); OP_FETCH; break;
      case 1770: // set 0,(hl) 15(4,4,4,3)
        break;
      case 1771:
        MEM_READ(regHL); break;
      case 1772:
        break;
      case 1773:
        regV = cpuData; break;
      case 1774:
        regV |= BIT(IRh); break;
      case 1775:
        MEM_WRITE(regHL, regV); break;
      case 1776:
        break;
      case 1777:
        OP_FETCH; break;
      case 1778: // set 0,a 8(4,4)
        regA |= BIT(IRh); OP_FETCH; break;
      case 1779: // rlc (ix/iy+D),b 23(4,4,3,5,4,3)
        ALU_RLC(); regB = regV; break;
      case 1780:
        MEM_WRITE(regWZ, regV); break;
      case 1781:
        break;
      case 1782:
        regF = regQ; OP_FETCH; break;
      case 1783: // rlc (ix+D),c 23(4,4,3,5,4,3)
        ALU_RLC(); regC = regV; CYCLEINC(-3); break;
      case 1784: // rlc (ix+D),d 23(4,4,3,5,4,3)
        ALU_RLC(); regD = regV; CYCLEINC(-4); break;
      case 1785: // rlc (ix/iy+D),e 23(4,4,3,5,4,3)
        ALU_RLC(); regE = regV; CYCLEINC(-5); break;
      case 1786: // rlc (ix/iy+D),h 23(4,4,3,5,4,3)
        ALU_RLC(); regH = regV; CYCLEINC(-6); break;
      case 1787: // rlc (ix/iy+D),l 23(4,4,3,5,4,3)
        ALU_RLC(); regL = regV; CYCLEINC(-7); break;
      case 1788: // rlc (ix/iy+D) 23(4,4,3,5,4,3)
        ALU_RLC(); CYCLEINC(-8); break;
      case 1789: // rlc (ix/iy+D),a 23(4,4,3,5,4,3)
        ALU_RLC(); regA = regV; CYCLEINC(-9); break;
      case 1790: // rrc (ix/iy+D),b 23(4,4,3,5,4,3)
        ALU_RRC(); regB = regV; break;
      case 1791:
        MEM_WRITE(regWZ, regV); break;
      case 1792:
        break;
      case 1793:
        regF = regQ; OP_FETCH; break;
      case 1794: // rrc (ix/iy+D),c 23(4,4,3,5,4,3)
        ALU_RRC(); regC = regV; CYCLEINC(-3); break;
      case 1795: // rrc (ix/iy+D),d 23(4,4,3,5,4,3)
        ALU_RRC(); regD = regV; CYCLEINC(-4); break;
      case 1796: // rrc (ix/iy+D),e 23(4,4,3,5,4,3)
        ALU_RRC(); regE = regV; CYCLEINC(-5); break;
      case 1797: // rrc (ix/iy+D),h 23(4,4,3,5,4,3)
        ALU_RRC(); regH = regV; CYCLEINC(-6); break;
      case 1798: // rrc (ix/iy+D),l 23(4,4,3,5,4,3)
        ALU_RRC(); regL = regV; CYCLEINC(-7); break;
      case 1799: // rrc (ix/iy+D) 23(4,4,3,5,4,3)
        ALU_RRC(); CYCLEINC(-8); break;
      case 1800: // rrc (ix/iy+D),a 23(4,4,3,5,4,3)
        ALU_RRC(); regA = regV; CYCLEINC(-9); break;
      case 1801: // rl (ix/iy+D),b 23(4,4,3,5,4,3)
        ALU_RL(); regB = regV; break;
      case 1802:
        MEM_WRITE(regWZ, regV); break;
      case 1803:
        break;
      case 1804:
        regF = regQ; OP_FETCH; break;
      case 1805: // rl (ix/iy+D),c 23(4,4,3,5,4,3)
        ALU_RL(); regC = regV; CYCLEINC(-3); break;
      case 1806: // rl (ix/iy+D),d 23(4,4,3,5,4,3)
        ALU_RL(); regD = regV; CYCLEINC(-4); break;
      case 1807: // rl (ix/iy+D),e 23(4,4,3,5,4,3)
        ALU_RL(); regE = regV; CYCLEINC(-5); break;
      case 1808: // rl (ix/iy+D),h 23(4,4,3,5,4,3)
        ALU_RL(); regH = regV; CYCLEINC(-6); break;
      case 1809: // rl (ix/iy+D),l 23(4,4,3,5,4,3)
        ALU_RL(); regL = regV; CYCLEINC(-7); break;
      case 1810: // rl (ix/iy+D) 23(4,4,3,5,4,3)
        ALU_RL(); CYCLEINC(-8); break;
      case 1811: // rl (ix/iy+D),a 23(4,4,3,5,4,3)
        ALU_RL(); regA = regV; CYCLEINC(-9); break;
      case 1812: // rr (ix/iy+D),b 23(4,4,3,5,4,3)
        ALU_RR(); regB = regV; break;
      case 1813:
        MEM_WRITE(regWZ, regV); break;
      case 1814:
        break;
      case 1815:
        regF = regQ; OP_FETCH; break;
      case 1816: // rr (ix/iy+D),c 23(4,4,3,5,4,3)
        ALU_RR(); regC = regV; CYCLEINC(-3); break;
      case 1817: // rr (ix/iy+D),d 23(4,4,3,5,4,3)
        ALU_RR(); regD = regV; CYCLEINC(-4); break;
      case 1818: // rr (ix/iy+D),e 23(4,4,3,5,4,3)
        ALU_RR(); regE = regV; CYCLEINC(-5); break;
      case 1819: // rr (ix/iy+D),h 23(4,4,3,5,4,3)
        ALU_RR(); regH = regV; CYCLEINC(-6); break;
      case 1820: // rr (ix/iy+D),l 23(4,4,3,5,4,3)
        ALU_RR(); regL = regV; CYCLEINC(-7); break;
      case 1821: // rr (ix/iy+D) 23(4,4,3,5,4,3)
        ALU_RR(); CYCLEINC(-8); break;
      case 1822: // rr (ix/iy+D),a 23(4,4,3,5,4,3)
        ALU_RR(); regA = regV; CYCLEINC(-9); break;
      case 1823: // sla (ix/iy+D),b 23(4,4,3,5,4,3)
        ALU_SLA(); regB = regV; break;
      case 1824:
        MEM_WRITE(regWZ, regV); break;
      case 1825:
        break;
      case 1826:
        regF = regQ; OP_FETCH; break;
      case 1827: // sla (ix/iy+D),c 23(4,4,3,5,4,3)
        ALU_SLA(); regC = regV; CYCLEINC(-3); break;
      case 1828: // sla (ix/iy+D),d 23(4,4,3,5,4,3)
        ALU_SLA(); regD = regV; CYCLEINC(-4); break;
      case 1829: // sla (ix/iy+D),e 23(4,4,3,5,4,3)
        ALU_SLA(); regE = regV; CYCLEINC(-5); break;
      case 1830: // sla (ix/iy+D),h 23(4,4,3,5,4,3)
        ALU_SLA(); regH = regV; CYCLEINC(-6); break;
      case 1831: // sla (ix/iy+D),l 23(4,4,3,5,4,3)
        ALU_SLA(); regL = regV; CYCLEINC(-7); break;
      case 1832: // sla (ix/iy+D) 23(4,4,3,5,4,3)
        ALU_SLA(); CYCLEINC(-8); break;
      case 1833: // sla (ix/iy+D),a 23(4,4,3,5,4,3)
        ALU_SLA(); regA = regV; CYCLEINC(-9); break;
      case 1834: // sra (ix/iy+D),b 23(4,4,3,5,4,3)
        ALU_SRA(); regB = regV; break;
      case 1835:
        MEM_WRITE(regWZ, regV); break;
      case 1836:
        break;
      case 1837:
        regF = regQ; OP_FETCH; break;
      case 1838: // sra (ix/iy+D),c 23(4,4,3,5,4,3)
        ALU_SRA(); regC = regV; CYCLEINC(-3); break;
      case 1839: // sra (ix/iy+D),d 23(4,4,3,5,4,3)
        ALU_SRA(); regD = regV; CYCLEINC(-4); break;
      case 1840: // sra (ix/iy+D),e 23(4,4,3,5,4,3)
        ALU_SRA(); regE = regV; CYCLEINC(-5); break;
      case 1841: // sra (ix/iy+D),h 23(4,4,3,5,4,3)
        ALU_SRA(); regH = regV; CYCLEINC(-6); break;
      case 1842: // sra (ix/iy+D),l 23(4,4,3,5,4,3)
        ALU_SRA(); regL = regV; CYCLEINC(-7); break;
      case 1843: // sra (ix/iy+D) 23(4,4,3,5,4,3)
        ALU_SRA(); CYCLEINC(-8); break;
      case 1844: // sra (ix/iy+D),a 23(4,4,3,5,4,3)
        ALU_SRA(); regA = regV; CYCLEINC(-9); break;
      case 1845: // sll (ix/iy+D),b 23(4,4,3,5,4,3)
        ALU_SLL(); regB = regV; break;
      case 1846:
        MEM_WRITE(regWZ, regV); break;
      case 1847:
        break;
      case 1848:
        regF = regQ; OP_FETCH; break;
      case 1849: // sll (ix/iy+D),c 23(4,4,3,5,4,3)
        ALU_SLL(); regC = regV; CYCLEINC(-3); break;
      case 1850: // sll (ix/iy+D),d 23(4,4,3,5,4,3)
        ALU_SLL(); regD = regV; CYCLEINC(-4); break;
      case 1851: // sll (ix/iy+D),e 23(4,4,3,5,4,3)
        ALU_SLL(); regE = regV; CYCLEINC(-5); break;
      case 1852: // sll (ix/iy+D),h 23(4,4,3,5,4,3)
        ALU_SLL(); regH = regV; CYCLEINC(-6); break;
      case 1853: // sll (ix/iy+D),l 23(4,4,3,5,4,3)
        ALU_SLL(); regL = regV; CYCLEINC(-7); break;
      case 1854: // sll (ix/iy+D) 23(4,4,3,5,4,3)
        ALU_SLL(); CYCLEINC(-8); break;
      case 1855: // sll (ix/iy+D),a 23(4,4,3,5,4,3)
        ALU_SLL(); regA = regV; CYCLEINC(-9); break;
      case 1856: // srl (ix/iy+D),b 23(4,4,3,5,4,3)
        ALU_SRL(); regB = regV; break;
      case 1857:
        MEM_WRITE(regWZ, regV); break;
      case 1858:
        break;
      case 1859:
        regF = regQ; OP_FETCH; break;
      case 1860: // srl (ix/iy+D),c 23(4,4,3,5,4,3)
        ALU_SRL(); regC = regV; CYCLEINC(-3); break;
      case 1861: // srl (ix/iy+D),d 23(4,4,3,5,4,3)
        ALU_SRL(); regD = regV; CYCLEINC(-4); break;
      case 1862: // srl (ix/iy+D),e 23(4,4,3,5,4,3)
        ALU_SRL(); regE = regV; CYCLEINC(-5); break;
      case 1863: // srl (ix/iy+D),h 23(4,4,3,5,4,3)
        ALU_SRL(); regH = regV; CYCLEINC(-6); break;
      case 1864: // srl (ix/iy+D),l 23(4,4,3,5,4,3)
        ALU_SRL(); regL = regV; CYCLEINC(-7); break;
      case 1865: // srl (ix/iy+D) 23(4,4,3,5,4,3)
        ALU_SRL(); CYCLEINC(-8); break;
      case 1866: // srl (ix/iy+D),a 23(4,4,3,5,4,3)
        ALU_SRL(); regA = regV; CYCLEINC(-9); break;
      case 1867: // bit 0,(ix/iy+D) 20(4,4,3,5,4)
        ALU_Bit(); OP_FETCH; break;
      case 1868: // res 0,(ix/iy+D),b 23(4,4,3,5,4,3)
        regV &= ~BIT(IRh); regB = regV; break;
      case 1869:
        MEM_WRITE(regWZ, regV); break;
      case 1870:
        break;
      case 1871:
        OP_FETCH; break;
      case 1872: // res 0,(ix/iy+D),c 23(4,4,3,5,4,3)
        regV &= ~BIT(IRh); regC = regV; CYCLEINC(-3); break;
      case 1873: // res 0,(ix/iy+D),d 23(4,4,3,5,4,3)
        regV &= ~BIT(IRh); regD = regV; CYCLEINC(-4); break;
      case 1874: // res 0,(ix/iy+D),e 23(4,4,3,5,4,3)
        regV &= ~BIT(IRh); regE = regV; CYCLEINC(-5); break;
      case 1875: // res 0,(ix/iy+D),h 23(4,4,3,5,4,3)
        regV &= ~BIT(IRh); regH = regV; CYCLEINC(-6); break;
      case 1876: // res 0,(ix/iy+D),l 23(4,4,3,5,4,3)
        regV &= ~BIT(IRh); regL = regV; CYCLEINC(-7); break;
      case 1877: // res 0,(ix/iy+D) 23(4,4,3,5,4,3)
        regV &= ~BIT(IRh); CYCLEINC(-8); break;
      case 1878: // res 0,(ix/iy+D),a 23(4,4,3,5,4,3)
        regV &= ~BIT(IRh); regA = regV; CYCLEINC(-9); break;
      case 1879: // set 0,(ix/iy+D),b 23(4,4,3,5,4,3)
        regV |= BIT(IRh); regB = regV; break;
      case 1880:
        MEM_WRITE(regWZ, regV); break;
      case 1881:
        break;
      case 1882:
        OP_FETCH; break;
      case 1883: // set 0,(ix/iy+D),c 23(4,4,3,5,4,3)
        regV |= BIT(IRh); regC = regV; CYCLEINC(-3); break;
      case 1884: // set 0,(ix/iy+D),d 23(4,4,3,5,4,3)
        regV |= BIT(IRh); regD = regV; CYCLEINC(-4); break;
      case 1885: // set 0,(ix/iy+D),e 23(4,4,3,5,4,3)
        regV |= BIT(IRh); regE = regV; CYCLEINC(-5); break;
      case 1886: // set 0,(ix/iy+D),h 23(4,4,3,5,4,3)
        regV |= BIT(IRh); regH = regV; CYCLEINC(-6); break;
      case 1887: // set 0,(ix/iy+D),l 23(4,4,3,5,4,3)
        regV |= BIT(IRh); regL = regV; CYCLEINC(-7); break;
      case 1888: // set 0,(ix/iy+D) 23(4,4,3,5,4,3)
        regV |= BIT(IRh); CYCLEINC(-8); break;
      case 1889: // set 0,(ix/iy+D),a 23(4,4,3,5,4,3)
        regV |= BIT(IRh); regA = regV; CYCLEINC(-9); break;
    }

    CPU_IORQ = iorq > 0;
    CPU_MREQ = mreq > 0;
}
