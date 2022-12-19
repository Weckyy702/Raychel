from math import *

C = 128
R = 3.442619855899
V = 9.91256303526217e-3

zig_x = [0 for i in range(C+1)]
zig_r = [0 for i in range(C)]

#C code:
#static void zigNorInit(int iC, double dR, double dV)
#{
#   int i; double f;
#   f = exp(-0.5 * R * R);
#   zig_x[0] = V / f;
#   zig_x[1] = R;
#   zig_x[iC] = 0;
#   for (i = 2; i < C; ++i)
#   {
#       zig_x[i] = sqrt(-2 * log(V / zig_x[i - 1] + f));
#       f = exp(-0.5 * zig_x[i] * zig_x[i]);
#   }
#   for (i = 0; i < C; ++i)
#       zig_r[i] = zig_x[i + 1] / zig_x[i];
#}

def main():
    f = exp(-0.5 * R * R)
    zig_x[0] = V / f
    zig_x[1] = R
    zig_x[C] = 0
    for i in range(2, C):
        zig_x[i] = sqrt(-2 * log(V / zig_x[i - 1] + f))
        f = exp(-0.5 * zig_x[i] * zig_x[i])
    for i in range(C):
        zig_r[i] = zig_x[i + 1] / zig_x[i]

    print("X:")
    for x in zig_x:
        print(f"{x}, ")

    print("R: ")
    for r in zig_r:
        print(f"{r}, ")

if __name__ == "__main__":
    main()
