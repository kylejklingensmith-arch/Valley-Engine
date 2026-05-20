#include "Valley/Terrain/Biome.h"

#include <utility>

namespace Valley::Terrain {

BiomeClassifier::BiomeClassifier(std::vector<BiomeDefinition> definitions)
    : m_definitions(std::move(definitions))
{
}

BiomeId BiomeClassifier::classify(double height_meters, double moisture, double temperature) const
{
    for (const auto& definition : m_definitions) {
        if (height_meters >= definition.min_height_meters && height_meters < definition.max_height_meters
            && moisture >= definition.min_moisture && moisture <= definition.max_moisture
            && temperature >= definition.min_temperature && temperature <= definition.max_temperature) {
            return definition.id;
        }
    }
    return BiomeId::Grassland;
}

const std::vector<BiomeDefinition>& BiomeClassifier::definitions() const
{
    return m_definitions;
}

std::vector<BiomeDefinition> BiomeClassifier::default_definitions()
{
    return {
        { .id = BiomeId::Water, .min_height_meters = -1000.0, .max_height_meters = -8.0 },
        { .id = BiomeId::Snow, .min_height_meters = 95.0, .max_height_meters = 1000.0, .min_temperature = 0.0, .max_temperature = 0.45 },
        { .id = BiomeId::Highlands, .min_height_meters = 70.0, .max_height_meters = 1000.0 },
        { .id = BiomeId::Forest, .min_height_meters = -8.0, .max_height_meters = 95.0, .min_moisture = 0.55, .max_moisture = 1.0 },
        { .id = BiomeId::Grassland, .min_height_meters = -8.0, .max_height_meters = 95.0 },
    };
}

} // namespace Valley::Terrain
