#include <SDL.h>
#include <vector>
#include "Scene.h"
#include "ColorRGB.h"

namespace sdf
{

    struct ResultStats;

	class Renderer final
    {
    public:
        Renderer(uint32_t const& width, uint32_t const& height);
        ~Renderer();

        void Render(Scene const& pScene) const;
        bool SaveBufferToImage(std::string const& imageName) const;

		ResultStats GetCollisionStats(bool miss) const;

		glm::ivec2 GetWindowDimensions() const;
    private:
        void CalculateHitRecords(Scene const& pScene, float fovValue, glm::vec3 const& cameraOrigin, glm::mat3 const& cameraToWorld, uint32_t pixelIdx) const;
        static ColorRGB Palette(float distance);

        uint32_t m_Width;
        uint32_t m_Height;
        float m_AspectRatio;
        SDL_Window* m_WindowPtr;
        SDL_Renderer* m_RendererPtr;
        SDL_Texture* m_TexturePtr;
        std::vector<uint32_t> m_PixelIndices;
        mutable std::vector<uint32_t> m_PixelVec{};
		SDL_PixelFormat* m_PixelFormatPtr;
        mutable std::vector<HitRecord> m_HitRecordVec;
    };
}