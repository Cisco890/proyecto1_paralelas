#include "tulip.hpp"

#include <algorithm>
#include <thread>

namespace {
void updateTulipRange(std::vector<Tulip>& tulips,
                      const TulipTextures& textures,
                      int screenWidth, int screenHeight, int groundY,
                      std::size_t begin, std::size_t end) {
    for (std::size_t index = begin; index < end; ++index) {
        const FlowerTextures& selected = tulips[index].orange ?
            textures.orange : textures.red;
        updateFlowerPosition(tulips[index].placement, selected,
                             screenWidth, screenHeight, groundY);
    }
}
}

bool loadTulipTextures(TulipTextures& textures, SDL_Renderer* renderer) {
    textures = {};
    return loadFlowerTextures(textures.red, renderer,
                              "assets/flowers/red_tulip_frame_1.png",
                              "assets/flowers/red_tulip_frame_2.png") &&
           loadFlowerTextures(textures.orange, renderer,
                              "assets/flowers/orange_tulip_frame_1.png",
                              "assets/flowers/orange_tulip_frame_2.png");
}

void destroyTulipTextures(TulipTextures& textures) {
    destroyFlowerTextures(textures.red);
    destroyFlowerTextures(textures.orange);
}

std::vector<Tulip> createTulipField(std::size_t count) {
    std::vector<Tulip> tulips;
    tulips.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        tulips.push_back({
            {{}, 0.04f + static_cast<float>((index * 47) % 91) / 100.0f,
             0.20f + static_cast<float>((index * 29) % 70) / 100.0f,
             static_cast<Uint32>((index % 2) * 175)},
            index % 2 == 1
        });
    }
    return tulips;
}

void updateTulipPositionsSequential(std::vector<Tulip>& tulips,
                                    const TulipTextures& textures,
                                    int screenWidth, int screenHeight, int groundY) {
    updateTulipRange(
        tulips, textures, screenWidth, screenHeight, groundY, 0, tulips.size()
    );
}

void updateTulipPositionsParallel(std::vector<Tulip>& tulips,
                                  const TulipTextures& textures,
                                  int screenWidth, int screenHeight, int groundY) {
    if (tulips.empty()) return;
    const unsigned int available = std::thread::hardware_concurrency();
    const std::size_t workerCount = std::min<std::size_t>(
        available == 0 ? 2 : available, tulips.size());
    const std::size_t chunk = (tulips.size() + workerCount - 1) / workerCount;
    std::vector<std::thread> workers;
    for (std::size_t worker = 0; worker < workerCount; ++worker) {
        const std::size_t begin = worker * chunk;
        const std::size_t end = std::min(begin + chunk, tulips.size());
        workers.emplace_back([&, begin, end]() {
            updateTulipRange(
                tulips, textures, screenWidth, screenHeight, groundY, begin, end
            );
        });
    }
    for (std::thread& worker : workers) worker.join();
}

void renderTulip(SDL_Renderer* renderer, const TulipTextures& textures,
                 const Tulip& tulip, Uint32 currentTicks, Uint8 opacity) {
    renderFlower(renderer, tulip.orange ? textures.orange : textures.red,
                 tulip.placement, currentTicks, opacity);
}
