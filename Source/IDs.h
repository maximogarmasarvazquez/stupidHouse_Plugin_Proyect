#pragma once

namespace IDs
{
    /* --- Shape / Heat / Spice / Depth ---------------------------------- */
    constexpr auto shapePreset = "shapePreset";
    constexpr auto shape = "shape";
    constexpr auto dryWetDistortion = "dryWetDistortion";

    constexpr auto heatPreset = "heatPreset";
    constexpr auto heat = "heat";
    constexpr auto spicePreset = "spicePreset";
    constexpr auto spice = "spice";
    constexpr auto depthPreset = "depthPreset";
    constexpr auto depth = "depth";


    /* --- Macro ---------------------------------------------------------- */
    constexpr auto overall = "overall";      // macro de saturación

    /* --- Delay ---------------------------------------------------------- */
    constexpr auto time = "time";
    constexpr auto feedback = "feedback";
    constexpr auto dryWetDelay = "dryWetDelay";

    /* --- Modulación ----------------------------------------------------- */
    constexpr auto speed = "speed";
    constexpr auto dryWetMod = "dryWetMod";

    /* --- EQ / Salida ---------------------------------------------------- */
    constexpr auto highShelf = "highShelf";
    constexpr auto outputGain = "outputGain";   // ganancia final ♠︎ (SUSTITUYE al 2.º “overall” duplicado)
}