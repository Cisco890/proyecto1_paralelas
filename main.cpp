#include <SDL2/SDL.h>
#include <iostream>

// Dimensiones de la ventana
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int argc, char* argv[]) {

    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Error al inicializar SDL: "
                  << SDL_GetError() << std::endl;
        return 1;
    }

    // Crear ventana
    SDL_Window* window = SDL_CreateWindow(
        "Arbol Estacional - Screen Saver",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (window == nullptr) {
        std::cerr << "Error al crear la ventana: "
                  << SDL_GetError() << std::endl;

        SDL_Quit();
        return 1;
    }

    // Crear renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (renderer == nullptr) {
        std::cerr << "Error al crear el renderer: "
                  << SDL_GetError() << std::endl;

        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Rectangulo que representa el tronco
    SDL_Rect trunk;

    trunk.w = 80;
    trunk.h = 250;

    // Centrar horizontalmente
    trunk.x = (SCREEN_WIDTH - trunk.w) / 2;

    // Colocarlo sobre la parte inferior de la pantalla
    trunk.y = SCREEN_HEIGHT - trunk.h;

    bool running = true;

    // Loop principal
    while (running) {

        
        // 1. Eventos
        

        SDL_Event event;

        while (SDL_PollEvent(&event)) {

            // Cerrar ventana
            if (event.type == SDL_QUIT) {
                running = false;
            }

            // Salir presionando ESC
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }
        }

        
        // 2. Limpiar pantalla
        

        // Fondo azul claro
        SDL_SetRenderDrawColor(
            renderer,
            135, 206, 235, // RGB
            255
        );

        SDL_RenderClear(renderer);

        
        // 3. Dibujar tronco
        

        // Color cafe
        SDL_SetRenderDrawColor(
            renderer,
            120, 72, 40, // RGB
            255
        );

        SDL_RenderFillRect(renderer, &trunk);

        
        // 4. Mostrar frame
        

        SDL_RenderPresent(renderer);
    }

    
    // Liberar recursos
    

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}