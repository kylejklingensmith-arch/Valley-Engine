#pragma once

#include "Valley/Terrain/TerrainTypes.h"

#include <vector>

namespace Valley::Terrain {

struct BiomeDefinition {
    BiomeId id = BiomeId::Grassland;
    double min_height_meters = -1000.0;
    double max_height_meters = 1000.0;
    double min_moisture = 0.0;
    double max_moisture = 1.0;
    double min_temperature = 0.0;
    double max_temperature = 1.0;
};

class BiomeClassifier {
public:
    explicit BiomeClassifier(std::vector<BiomeDefinition> definitions = default_definitions());

    [[nodiscard]] BiomeId classify(double height_meters, double moisture, double temperature) const;
    [[nodiscard]] const std::vector<BiomeDefinition>& definitions() const;

    [[nodiscard]] static std::vector<BiomeDefinition> default_definitions();

private:
    std::vector<BiomeDefinition> m_definitions;
};

} // namespace Valley::Terrain
