# Algoritmo SaP

# Dependecias
-LexRis Logic Headers v2.10. -> https://github.com/chanochambure/LexRisLogicHeaders

-Allegro 5.2.0.

# Compilación
g++ -std=c++11 main.cpp -lLL -LL_Allegro5 -lLL_MathStructure -o SaP.out

# Ejecución
./SaP.out Display_Size_X Display_Size_Y Object_Space Elements Tests

./SaP.out 1366 768 0.2 1024 100000

# Controles:
Make Collision: C

Show Polygon: P

Show Frames: F

Render Object: S

Add Random Object: A

Change Position: O

Print Collision Info: SPACE
