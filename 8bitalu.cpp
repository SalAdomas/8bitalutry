#include <iostream>
#include "decoder.h"
using namespace std;

bool NAND(bool x, bool y)
{
    if (x == 1 && y == 1)
        return 0;
    else
        return 1;
}

bool OR(bool x, bool y) {
    return NAND(NAND(x, x), NAND(y, y));
}

bool NOT(bool x) {
    return NAND(x, x);
}

bool AND(bool x, bool y) {
    return NOT(NAND(x, y));
}

bool XOR(bool x, bool y) {
    return NAND(NAND(NAND(x, y), x), NAND(NAND(x, y), y));
}

bool NOR(bool x, bool y) {
    return NAND(NAND(NAND(x, x), NAND(y, y)), NAND(NAND(x, x), NAND(y, y)));
}

void HALFADDER(bool x, bool y, bool &sum, bool &carry)
{
    sum = XOR(x, y);
    carry = AND(x, y);
}

void FULLADDER(bool x, bool y, bool carryin, bool &carryout, bool &suma)
{
    bool cr1, cr2, sum1, sum2;
    HALFADDER(x, y, sum1, cr1);
    HALFADDER(sum1, carryin, sum2, cr2);
    carryout = OR(cr1, cr2);
    suma = sum2;
}

void muxas(bool M0, bool M1, bool &nn, bool &nv, bool &vn, bool &vv) {
    nn = AND(NOT(M0), NOT(M1));
    nv = AND(NOT(M0), M1);
    vn = AND(M0, NOT(M1));
    vv = AND(M0, M1);
}

// Nauja funkcija, atliekanti multiplexerio pasirinkimą
void selectOp(bool A, bool B, bool selNotA, bool selNotB, bool selXor, bool selAdd, bool &result, bool &sumVal) {
    bool opNotA = NOT(A);
    bool opNotB = NOT(B);
    bool opXor  = XOR(A, B);
    result = (selNotA && opNotA) ||
             (selNotB && opNotB) ||
             (selXor  && opXor)  ||
             (selAdd  && sumVal);
}

void ONEBITALU(bool A, bool B, bool ENA, bool ENB, bool M0, bool M1, bool CarryIn, bool &CarryOut, bool &Output)
{
    bool a = AND(ENA, A);
    bool b = AND(ENB, B);
    bool nn, nv, vn, vv, out, suma;
    muxas(M0, M1, nn, nv, vn, vv);
    FULLADDER(a, b, CarryIn, CarryOut, suma);
    // Pasirenkame operaciją naudodami naują multiplexerio funkciją
    selectOp(a, b, nn, nv, vn, vv, out, suma);
    Output = out;
}

void EIGHTBITALU(bool A[], bool B[], bool output[], bool d0, bool d1, bool d2, bool cntrl, bool &cflag, bool &truefalse) {
    bool no, negb, lyg, plius, daug, shift;
    bool carryin, carryout, out;
    bool sift[8] = { false };

    // Dekoderis iš 3 bitų į 6 valdymo signalus
    // NO -> nera operacijos, negb -> -B, lyg -> A == B, plius -> A+B, daug -> daugyba, shift -> poslinkis
    decoder(d0, d1, d2, no, negb, lyg, plius, daug, shift);

    if (no) {
        for (int i = 0; i < 8; i++)
            output[i] = 0;
        return;
    }

    if (negb) {
        carryin = 1;
        for (int i = 7; i >= 0; i--) {
            ONEBITALU(A[i], B[i], false, true, false, true, carryin, carryout, output[i]);
            carryin = carryout;
            cflag = carryout;
        }
        return;
    }

    if (lyg) {
        bool result = 1;
        for (int i = 0; i < 8; i++) {
            ONEBITALU(A[i], B[i], true, true, true, false, false, carryout, out);
            result = AND(result, NOT(out));
        }
        truefalse = 1;
        cout << (result ? "TRUE" : "FALSE") << endl;
        return;
    }

    if (plius) {
        carryin = 0;
        for (int i = 7; i >= 0; i--) {
            ONEBITALU(A[i], B[i], true, true, true, true, carryin, carryout, out);
            output[i] = out;
            carryin = carryout;
            cflag = carryout;
        }
        return;
    }

    if (shift) {
        poslinkis(A, cntrl, output);
        return;
    }

    if (daug) {
        bool keitaliojantisB[8];
        for (int i = 0; i < 8; i++)
            keitaliojantisB[i] = B[i];
        carryin = 0;
        for (int i = 7; i >= 0; i--) {
            if (A[i])
                ONEBITALU(A[i], keitaliojantisB[i], true, true, true, true, carryin, carryout, out);
            else
                ONEBITALU(A[i], false, true, true, true, true, carryin, carryout, out);
            carryin = carryout;
            output[i] = out;
            poslinkis(keitaliojantisB, false, sift);
            for (int j = 0; j < 8; j++)
                keitaliojantisB[j] = sift[j];
            keitaliojantisB[7] = false;
        }
    }
}

void decoder(bool d0, bool d1, bool d2, bool &NO, bool &negb, bool &lyg, bool &plius, bool &daug, bool &shift) {
    NO = OR(OR(AND(AND(NOT(d0), NOT(d1)), NOT(d2)), AND(AND(d0, d1), NOT(d2))), AND(AND(d0, d1), d2));
    negb = AND(AND(NOT(d0), NOT(d1)), d2);
    lyg = AND(AND(NOT(d0), d1), NOT(d2));
    plius = AND(AND(NOT(d0), d1), d2);
    daug = AND(AND(d0, NOT(d1)), NOT(d2));
    shift = AND(AND(d0, NOT(d1)), d2);
}

int main() {
    bool A[8] = { 0,0,0,0,0,0,1,0 };
    bool B[8] = { 0,0,0,0,0,0,1,1 };
    bool O[8] = { false };

    bool d0 = 0, d1 = 1, d2 = 1;
    bool cntrl = 0;
    bool cflag = 0, truefalse = 0;

    EIGHTBITALU(A, B, O, d0, d1, d2, cntrl, cflag, truefalse);

    if (!truefalse) {
        cout << "Rezultatas: ";
        for (int i = 0; i < 8; i++)
            cout << O[i];
        cout << "\nCarry flag (Cflag): " << cflag << endl;
    }

    return 0;
}
