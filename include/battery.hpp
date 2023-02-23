#pragma once

namespace battery {

    // Initializes the battery management system
    int init();

    // Gets the current battery percentage
    // Positive values indicate half battery percentages (200 = 100%, 100 = 50%, etc..), negative values an error
    int get_percentage();

}