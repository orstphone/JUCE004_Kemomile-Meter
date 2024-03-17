/*
  ==============================================================================

    SecondOrderIIRFilter.cpp
    Created: 17 Mar 2024 1:40:44pm
     Author:  Decca (https://github.com/deccauvw)

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SecondOrderIIRFilter.h"

//==============================================================================
SecondOrderIIRFilter::SecondOrderIIRFilter(
    double b0_at48k_ = 1.0,
    double b1_at48k_ = 0.0,
    double b2_at48k_ = 0.0,
    double a1_at48k_ = 0.0,
    double a2_at48k_ = 0.0)
    :
    b0{ b0_at48k_ },
    b1{ b1_at48k_ },
    b2{ b2_at48k_ },
    a1{ a1_at48k_ },
    a2{ a2_at48k_ },
    numberOfChannels{ 0 }
{
    //determine the values: Q, VH, VB, VL, and arctanK
    const double K_over_Q = (2. - 2. * a2_at48k + 1.) / (a2_at48k - a1_at48k + 1.);
    const double K = sqrt((a1_at48k + a2_at48k + 1.) / (a2_at48k - a1_at48k + 1.));
    Q = K / K_over_Q;
    arctanK = atan(K);

    VB = (b0_at48k - b2_at48k) / (1. - a2_at48k);
    VH = (b0_at48k - b1_at48k + b2_at48k) / (a2_at48k - a1_at48k + 1.);
    VL = (b0_at48k + b1_at48k + b2_at48k) / (a1_at48k + a2_at48k + 1.);
}

SecondOrderIIRFilter::~SecondOrderIIRFilter()
{
}
