g++ -std=c++11 Skybox.cpp glad/glad.c learnOpenGL/stb_image_compile.cpp -o Skybox.exec -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

g++ -std=c++11 Skybox_edited.cpp learnOpenGL/stb_image_compile.cpp -o v0.exec -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

g++ -std=c++11 main.cpp stb/stb_image.cpp -o main.exec -lglfw3 -lassimp -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

g++ -std=c++11 main.cpp stb/stb_image.cpp -o main.exec -L./imgui -limgui -lglad -lglfw3 -lassimp -lfftw3f -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

g++ -std=c++11 main.cpp stb/stb_image.cpp -I ./ -o main.exec -lglad -lglfw3 -lassimp -lfftw3f -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

g++ -std=c++11 main.cpp stb/stb_image.cpp -I /usr/local/include/freetype2 -I ./ -o main.exec -lglad -lglfw3 -lassimp -lfftw3f -lfreetype -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

g++ -std=c++11 main.cpp stb/stb_image.cpp Triangle.cpp Mesh.cpp Model.cpp InputHandler.cpp ForShader.cpp -I /usr/local/include/freetype2 -o main.exec -lglad -lglfw3 -lassimp -lfftw3f -lfreetype -lIL -lILU -lILUT -lSDL2_mixer -lSDL2 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo