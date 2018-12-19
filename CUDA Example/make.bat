nvcc -ccbin "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin"^
    -I "C:\Bibliotecas MSVC\Allegro 5.1.12\include","C:\Users\Christian Benavides\Documents\Chano\Desarrollo\CUDA Example\LexRisLogic\include"^
    -L "C:\Bibliotecas MSVC\Allegro 5.1.12\lib" -lallegro_monolith cuda.cu^
         LexRisLogic\src\Allegro5\Allegro5.cpp^
         LexRisLogic\src\Allegro5\Display.cpp^
         LexRisLogic\src\Allegro5\Color.cpp^
         LexRisLogic\src\Allegro5\Input.cpp^
         LexRisLogic\src\Allegro5\Text.cpp^
         LexRisLogic\src\Allegro5\Timer.cpp^
         LexRisLogic\src\Allegro5\Primitives.cpp^
         LexRisLogic\src\MathStructures\Point.cpp^
         LexRisLogic\src\MathStructures\LineSegment.cpp^
         LexRisLogic\src\MathStructures\Polygon.cpp^
         LexRisLogic\src\MathStructures\MBB.cpp^
         LexRisLogic\src\Math.cpp^
         LexRisLogic\src\Convert.cpp^
         LexRisLogic\src\StringSplitter.cpp^
         LexRisLogic\src\FileStream.cpp^
         LexRisLogic\src\Time.cpp^
    -o cuda.exe