#include <iostream>
#include <cstring>

using namespace std;

#define Move(source, target, piece, capture, doublePawn, ep, ca) \
(                          \
    (source) |             \
    (target << 7) |        \
    (piece << 14) |        \
    (capture << 18) |      \
    (doublePawn << 19) |         \
    (ep << 20) |    \
    (ca << 21)       \
)

#define getSource(move) (move & 0x7f)
#define getTarget(move) ((move >> 7) & 0x7f)
#define getPromoted(move) ((move >> 14) & 0xf)
#define getCapture(move) ((move >> 18) & 0x1)
#define getDoublePawn(move) ((move >> 19) & 0x1)
#define getEnpassant(move) ((move >> 20) & 0x1)
#define getCastling(move) ((move >> 21) & 0x1)

enum Piece { e, P, N, B, R, Q, K, p, n, b, r, q, k, o };
enum Side { white, black };
enum Castling { wk = 1, wq = 2, bk = 4, bq = 8 };

enum Square {
    a8 = 0,   b8, c8, d8, e8, f8, g8, h8,
    a7 = 16,  b7, c7, d7, e7, f7, g7, h7,
    a6 = 32,  b6, c6, d6, e6, f6, g6, h6,
    a5 = 48,  b5, c5, d5, e5, f5, g5, h5,
    a4 = 64,  b4, c4, d4, e4, f4, g4, h4,
    a3 = 80,  b3, c3, d3, e3, f3, g3, h3,
    a2 = 96,  b2, c2, d2, e2, f2, g2, h2,
    a1 = 112, b1, c1, d1, e1, f1, g1, h1, noSq
};

string notation[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", "i8", "j8", "k8", "l8", "m8", "n8", "o8", "p8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", "i7", "j7", "k7", "l7", "m7", "n7", "o7", "p7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", "i6", "j6", "k6", "l6", "m6", "n6", "o6", "p6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", "i5", "j5", "k5", "l5", "m5", "n5", "o5", "p5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4", "i4", "j4", "k4", "l4", "m4", "n4", "o4", "p4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", "i3", "j3", "k3", "l3", "m3", "n3", "o3", "p3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2", "i2", "j2", "k2", "l2", "m2", "n2", "o2", "p2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "i1", "j1", "k1", "l1", "m1", "n1", "o1", "p1"
};

int board[128] = {
    r, n, b, q, k, b, n, r,    0,  0,  5,  5,  0,  0,  5,  0, 
    p, p, p, p, p, p, p, p,    5,  5,  0,  0,  0,  0,  5,  5,
    e, e, e, e, e, e, e, e,    5, 10, 15, 20, 20, 15, 10,  5,
    e, e, e, e, e, e, e, e,    5, 10, 20, 30, 30, 20, 10,  5,    
    e, e, e, e, e, e, e, e,    5, 10, 20, 30, 30, 20, 10,  5,
    e, e, e, e, e, e, e, e,    5, 10, 15, 20, 20, 15, 10,  5,
    P, P, P, P, P, P, P, P,    5,  5,  0,  0,  0,  0,  5,  5,
    R, N, B, Q, K, B, N, R,    0,  0,  5,  5,  0,  0,  5,  0
};

int castlingRights[128] = {
     7, 15, 15, 15,  3, 15, 15, 11,  o, o, o, o, o, o, o, o,
    15, 15, 15, 15, 15, 15, 15, 15,  o, o, o, o, o, o, o, o,
    15, 15, 15, 15, 15, 15, 15, 15,  o, o, o, o, o, o, o, o,
    15, 15, 15, 15, 15, 15, 15, 15,  o, o, o, o, o, o, o, o,
    15, 15, 15, 15, 15, 15, 15, 15,  o, o, o, o, o, o, o, o,
    15, 15, 15, 15, 15, 15, 15, 15,  o, o, o, o, o, o, o, o,
    15, 15, 15, 15, 15, 15, 15, 15,  o, o, o, o, o, o, o, o,
    13, 15, 15, 15, 12, 15, 15, 14,  o, o, o, o, o, o, o, o
};

string pieces[] = {".", "♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};

int pieceWeights[] = { 0, 100, 300, 325, 500, 900, 10000, -100, -300, -325, -500, -900, -10000 };

int side = white;
int castling = 0;
int enpassant = 0;

uint64_t hashKey = 0ULL;

typedef struct {
    int moves[256];
    int count;
} MoveList;

typedef struct {
    int board[64][128];
    int kingSq[64][2];
    int side[64];
    int castling[64];
    int enpassant[64];
} Undo;

Undo undo[1];

int knightOffsets[8] = {33, 31, 18, 14, -33, -31, -18, -14};
int bishopOffsets[4] = {15, 17, -15, -17};
int rookOffsets[4] = {16, -16, 1, -1};
int kingOffsets[8] = {16, -16, 1, -1, 15, 17, -15, -17};

int kingSq[] = { e1, e8 };

long nodes = 0;

int ply = 0;

int pvTable[64][64];
int pvLength[64];

void PrintBoard()
{
    for(int sq = 0; sq < 128; sq++)
    {
        if (sq % 16 == 0) 
        {
            cout << endl;
            cout << 8 - (sq / 16) << " ";
        }

        if (!(sq & 0x88))
            cout << pieces[board[sq]] << " ";
    }
    
    cout << "\n  a b c d e f g h\n\n";
}

static inline bool isSqAttacked(int sq, int side)
{
    if (side == white)
    {
        if (!((sq + 17) & 0x88) && board[sq + 17] == P)
            return true;

        if (!((sq + 15) & 0x88) && board[sq + 15] == P)
            return true;
    }
    else
    {
        if (!((sq - 17) & 0x88) && board[sq - 17] == p)
            return true;

        if (!((sq - 15) & 0x88) && board[sq - 15] == p)
            return true;
    }

    for (int dir = 0; dir < 8; dir++)
    {
        int toSq = sq + knightOffsets[dir];

        if (!(toSq & 0x88))
            if (side == white ? board[toSq] == N : board[toSq] == n)
                return true;
    }

    for (int dir = 0; dir < 8; dir++)
    {
        int toSq = sq + kingOffsets[dir];

        if (!(toSq & 0x88))
            if (side == white ? board[toSq] == K : board[toSq] == k)
                return true;
    }

    for (int dir = 0; dir < 4; dir++)
    {
        int toSq = sq + bishopOffsets[dir];

        while (!(toSq & 0x88))
        {
            if (side == white ? board[toSq] == B || board[toSq] == Q : board[toSq] == b || board[toSq] == q)
                return true;

            if (board[toSq])
                break;

            toSq += bishopOffsets[dir];
        }
    }

    for (int dir = 0; dir < 4; dir++)
    {
        int toSq = sq + rookOffsets[dir];

        while (!(toSq & 0x88))
        {
            if (side == white ? board[toSq] == R || board[toSq] == Q : board[toSq] == r || board[toSq] == q)
                return true;

            if (board[toSq])
                break;

            toSq += rookOffsets[dir];
        }
    }

    return false;
}

static inline void AddMove(MoveList* moves, int move)
{
    moves->moves[moves->count++] = move;
}

static inline void GenerateMoves(MoveList* moves)
{
    moves->count = 0;

    for (int sq = 0; sq < 128; sq++)
    {
        if (!(sq & 0x88))
        {
            if (side == white)
            {
                if (board[sq] == P)
                {
                    int toSq = sq - 16;

                    if (!(toSq & 0x88) && !board[toSq])
                    {
                        if (sq >= a7 && sq <= h7)
                        {
                            AddMove(moves, Move(sq, toSq, Q, 0, 0, 0, 0));
                            AddMove(moves, Move(sq, toSq, R, 0, 0, 0, 0));
                            AddMove(moves, Move(sq, toSq, B, 0, 0, 0, 0));
                            AddMove(moves, Move(sq, toSq, N, 0, 0, 0, 0));
                        }
                        else
                            AddMove(moves, Move(sq, toSq, 0, 0, 0, 0, 0));

                        if (sq >= a2 && sq <= h2 && !board[toSq - 16])
                            AddMove(moves, Move(sq, toSq - 16, 0, 0, 1, 0, 0));
                    }

                    for (int dir = 0; dir < 4; dir++)
                    {
                        int pawnOffset = bishopOffsets[dir];

                        if (pawnOffset < 0)
                        {
                            int toSq = sq + pawnOffset;

                            if (!(toSq & 0x88))
                            {
                                if (sq >= a7 && sq <= h7 && board[toSq] >= 7 && board[toSq] <= 12)
                                {
                                    AddMove(moves, Move(sq, toSq, Q, 1, 0, 0, 0));
                                    AddMove(moves, Move(sq, toSq, R, 1, 0, 0, 0));
                                    AddMove(moves, Move(sq, toSq, B, 1, 0, 0, 0));
                                    AddMove(moves, Move(sq, toSq, N, 1, 0, 0, 0));
                                }
                                else
                                {
                                    if (board[toSq] >= 7 && board[toSq] <= 12)
                                        AddMove(moves, Move(sq, toSq, 0, 1, 0, 0, 0));
                                    
                                    if (toSq == enpassant)
                                        AddMove(moves, Move(sq, toSq, 0, 1, 0, 1, 0));
                                }
                            }
                        }
                    }
                }

                if (board[sq] == K)
                {
                    if (castling & wk)
                        if (!board[f1] && !board[g1])
                            if (!isSqAttacked(e1, black) && !isSqAttacked(f1, black))
                                AddMove(moves, Move(e1, g1, 0, 0, 0, 0, 1));

                    if (castling & wq)
                        if (!board[b1] && !board[c1] && !board[d1])
                            if (!isSqAttacked(e1, black) && !isSqAttacked(d1, black))
                                AddMove(moves, Move(e1, c1, 0, 0, 0, 0, 1));
                }
            }
            else
            {
                if (board[sq] == p)
                {
                    int toSq = sq + 16;

                    if (!(toSq & 0x88) && !board[toSq])
                    {
                        if (sq >= a2 && sq <= h2)
                        {
                            AddMove(moves, Move(sq, toSq, q, 0, 0, 0, 0));
                            AddMove(moves, Move(sq, toSq, r, 0, 0, 0, 0));
                            AddMove(moves, Move(sq, toSq, b, 0, 0, 0, 0));
                            AddMove(moves, Move(sq, toSq, n, 0, 0, 0, 0));
                        }
                        else
                            AddMove(moves, Move(sq, toSq, 0, 0, 0, 0, 0));

                        if (sq >= a7 && sq <= h7 && !board[toSq + 16])
                            AddMove(moves, Move(sq, toSq + 16, 0, 0, 1, 0, 0));
                    }

                    for (int dir = 0; dir < 4; dir++)
                    {
                        int pawnOffset = bishopOffsets[dir];

                        if (pawnOffset > 0)
                        {
                            int toSq = sq + pawnOffset;

                            if (!(toSq & 0x88))
                            {
                                if (sq >= a2 && sq <= h2 && board[toSq] >= 1 && board[toSq] <= 6)
                                {
                                    AddMove(moves, Move(sq, toSq, q, 1, 0, 0, 0));
                                    AddMove(moves, Move(sq, toSq, r, 1, 0, 0, 0));
                                    AddMove(moves, Move(sq, toSq, b, 1, 0, 0, 0));
                                    AddMove(moves, Move(sq, toSq, n, 1, 0, 0, 0));
                                }
                                else
                                {
                                    if (board[toSq] >= 1 && board[toSq] <= 6)
                                        AddMove(moves, Move(sq, toSq, 0, 1, 0, 0, 0));
                                    
                                    if (toSq == enpassant)
                                        AddMove(moves, Move(sq, toSq, 0, 1, 0, 1, 0));
                                }
                            }
                        }
                    }
                }

                if (board[sq] == k)
                {
                    if (castling & bk)
                        if (!board[f8] && !board[g8])
                            if (!isSqAttacked(e8, white) && !isSqAttacked(f8, white))
                                AddMove(moves, Move(e8, g8, 0, 0, 0, 0, 1));

                    if (castling & bq)
                        if (!board[b8] && !board[c8] && !board[d8])
                            if (!isSqAttacked(e8, white) && !isSqAttacked(d8, white))
                                AddMove(moves, Move(e8, c8, 0, 0, 0, 0, 1));
                }
            }

            if (side == white ? board[sq] == N : board[sq] == n)
            {
                for (int dir = 0; dir < 8; dir++)
                {
                    int toSq = sq + knightOffsets[dir];

                    if (!(toSq & 0x88))
                    {
                        int p = board[toSq];

                        if (side == white ? (!p || (p >= 7 && p <= 12)) : (!p || (p >= 1 && p <= 6)))
                        {
                            if (p)
                                AddMove(moves, Move(sq, toSq, 0, 1, 0, 0, 0));
                            else
                                AddMove(moves, Move(sq, toSq, 0, 0, 0, 0, 0));
                        }
                    }
                }
            }

            if (side == white ? board[sq] == K : board[sq] == k)
            {
                for (int dir = 0; dir < 8; dir++)
                {
                    int toSq = sq + kingOffsets[dir];

                    if (!(toSq & 0x88))
                    {
                        int p = board[toSq];

                        if (side == white ? (!p || (p >= 7 && p <= 12)) : (!p || (p >= 1 && p <= 6)))
                        {
                            if (p)
                                AddMove(moves, Move(sq, toSq, 0, 1, 0, 0, 0));
                            else
                                AddMove(moves, Move(sq, toSq, 0, 0, 0, 0, 0));
                        }
                    }
                }
            }

            if (side == white ? board[sq] == B || board[sq] == Q : board[sq] == n || board[sq] == q)
            {
                for (int dir = 0; dir < 4; dir++)
                {
                    int toSq = sq + bishopOffsets[dir];

                    while (!(toSq & 0x88))
                    {
                        int p = board[toSq];

                        if (side == white ? (p >= 1 && p <= 6) : (p >= 7 && p <= 12))
                            break;

                        if (side == white ? (p >= 7 && p <= 12) : (p >= 1 && p <= 6))
                        {
                            AddMove(moves, Move(sq, toSq, 0, 1, 0, 0, 0));
                            break;
                        }

                        if (!p)
                            AddMove(moves, Move(sq, toSq, 0, 0, 0, 0, 0));

                        toSq += bishopOffsets[dir];
                    }
                }
            }

            if (side == white ? board[sq] == R || board[sq] == Q : board[sq] == r || board[sq] == q)
            {
                for (int dir = 0; dir < 4; dir++)
                {
                    int toSq = sq + rookOffsets[dir];

                    while (!(toSq & 0x88))
                    {
                        int p = board[toSq];

                        if (side == white ? (p >= 1 && p <= 6) : (p >= 7 && p <= 12))
                            break;

                        if (side == white ? (p >= 7 && p <= 12) : (p >= 1 && p <= 6))
                        {
                            AddMove(moves, Move(sq, toSq, 0, 1, 0, 0, 0));
                            break;
                        }

                        if (!p)
                            AddMove(moves, Move(sq, toSq, 0, 0, 0, 0, 0));

                        toSq += rookOffsets[dir];
                    }
                }
            }
        }
    }
}

static inline void CopyBoard()
{
    memcpy(undo->board[ply], board, 512);
    memcpy(undo->kingSq[ply], kingSq, 8);
    
    undo->side[ply] = side;
    undo->castling[ply] = castling;
    undo->enpassant[ply] = enpassant;
}

static inline void TakeBack()
{
    memcpy(board, undo->board[ply], 512);
    memcpy(kingSq, undo->kingSq[ply], 8);
    
    side = undo->side[ply];
    castling = undo->castling[ply];
    enpassant = undo->enpassant[ply];
}

static inline int MakeMove(int move)
{
    CopyBoard();

    int from = getSource(move);
    int to = getTarget(move);
    int promoted = getPromoted(move);
    int doublePush = getDoublePawn(move);
    int ep = getEnpassant(move);
    int ca = getCastling(move);

    board[to] = board[from];
    board[from] = e;

    if (promoted)
        board[to] = promoted;

    if (ep)
        side == white ? board[to + 16] = e : board[to - 16] = e;

    enpassant = noSq;

    if (doublePush)
        side == white ? enpassant = to + 16 : enpassant = to - 16;

    if (ca)
    {
        switch (to)
        {
        case g1:
            board[f1] = board[h1];
            board[h1] = e;
            break;
        case c1:
            board[d1] = board[a1];
            board[a1] = e;
            break;
        case g8:
            board[f8] = board[h8];
            board[h8] = e;
            break;
        case c8:
            board[d8] = board[a8];
            board[a8] = e;
            break;
        }
    }

    if (board[to] == K || board[to] == k)
        kingSq[side] = to;

    castling &= castlingRights[from];
    castling &= castlingRights[to];

    side ^= 1;

    if (isSqAttacked(kingSq[side ^ 1], side))
    {
        TakeBack();
        return 0;
    }
    else
        return 1;
}

static inline void perft(int depth)
{
    if (depth == 0)
    {
        nodes++;
        return;
    }

    MoveList moves[1];
    GenerateMoves(moves);

    for (int i = 0; i < moves->count; i++)
    {
        CopyBoard();

        if(!MakeMove(moves->moves[i]))
            continue;

        ply++;

        perft(depth - 1);

        ply--;

        TakeBack();
    }
}

static inline int Eval()
{
    int score = 0;

    for (int sq = 0; sq < 128; sq++)
    {
        if (!(sq & 0x88))
        {
            if (board[sq])
            {
                score += pieceWeights[board[sq]];
                
                if (board[sq] >= 7 && board[sq] <= 12)
                    score += board[sq + 8];
                else
                    score -= board[sq + 8];
            }
        }
    }

    return score;
}

static inline int Negamax(int depth, int alpha, int beta)
{
    pvLength[ply] = ply;

    if (depth == 0)
        return Eval();

    MoveList moves[1];
    GenerateMoves(moves);

    int score = 0;

    for (int i = 0; i < moves->count; i++)
    {
        CopyBoard();

        if (!MakeMove(moves->moves[i]))
            continue;

        ply++;

        score = -Negamax(depth - 1, -beta, -alpha);

        ply--;

        TakeBack();

        if (score >= beta)
            return beta;

        if (score > alpha)
        {
            alpha = score;

            pvTable[ply][ply] = moves->moves[i];

            for (int nextPly = ply + 1; nextPly < pvLength[ply + 1]; nextPly++)
                pvTable[ply][nextPly] = pvTable[ply + 1][nextPly];

            pvLength[ply] = pvLength[ply + 1];
        }
    }

    return alpha;
}

int main()
{
	PrintBoard();

    int score = Negamax(4, -50000, 50000);

    cout << "Score: " << score << endl;
    cout << "Nodes: " << nodes << endl;
    cout << "BestMove: " << notation[getSource(pvTable[0][0])] << notation[getTarget(pvTable[0][0])] << endl;

	return 0;
}
