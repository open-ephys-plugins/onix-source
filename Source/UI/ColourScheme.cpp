//
//  ColourScheme.cpp
//  ProbeViewerPlugin
//
//  Created by Kelly Fox on 10/16/17.
//  Copyright © 2017 Allen Institute. All rights reserved.
//

#include "ColourScheme.h"

using namespace ColourScheme;

#pragma mark ColourScheme utility forward declarations -
namespace
{ // hidden from the outside world (true static and hidden)
    ColourSchemeId selectedColourScheme = ColourSchemeId::INFERNO;

    Colour colourFromInferno(float val);
    Colour colourFromMagma(float val);
    Colour colourFromPlasma(float val);
    Colour colourFromViridis(float val);
    Colour colourFromJet(float val);
} // namespace

#pragma mark - ColourScheme interface methods -
void ColourScheme::setColourScheme(ColourSchemeId colourScheme)
{
    selectedColourScheme = colourScheme;
}

Colour ColourScheme::getColourForNormalizedValue(float val)
{
    switch (selectedColourScheme)
    {
    case ColourSchemeId::INFERNO:
        return colourFromInferno(val);

    case ColourSchemeId::MAGMA:
        return colourFromMagma(val);

    case ColourSchemeId::PLASMA:
        return colourFromPlasma(val);

    case ColourSchemeId::VIRIDIS:
        return colourFromViridis(val);

    case ColourSchemeId::JET:
        return colourFromJet(val);
    }
}

Colour ColourScheme::getColourForNormalizedValueInScheme(float val, ColourSchemeId colourScheme)
{
    switch (colourScheme)
    {
    case ColourSchemeId::INFERNO:
        return colourFromInferno(val);

    case ColourSchemeId::MAGMA:
        return colourFromMagma(val);

    case ColourSchemeId::PLASMA:
        return colourFromPlasma(val);

    case ColourSchemeId::VIRIDIS:
        return colourFromViridis(val);

    case ColourSchemeId::JET:
        return colourFromJet(val);
    }
}

#pragma mark - ColourScheme utility definitions -
namespace
{
    Colour colourFromInferno(float val)
    {
        if (val <= 0.003906)
        {
            return Colour::fromFloatRGBA(0.001462, 0.000466, 0.013866, 1.0);
        }
        if (val <= 0.007812)
        {
            return Colour::fromFloatRGBA(0.0022669999999999999, 0.0012700000000000001, 0.01857, 1.0);
        }
        if (val <= 0.011719)
        {
            return Colour::fromFloatRGBA(0.0032989999999999998, 0.0022490000000000001, 0.024239, 1.0);
        }
        if (val <= 0.015625)
        {
            return Colour::fromFloatRGBA(0.0045469999999999998, 0.003392, 0.030908999999999999, 1.0);
        }
        if (val <= 0.019531)
        {
            return Colour::fromFloatRGBA(0.0060060000000000001, 0.004692, 0.038558000000000002, 1.0);
        }
        if (val <= 0.023438)
        {
            return Colour::fromFloatRGBA(0.0076759999999999997, 0.006136, 0.046836000000000003, 1.0);
        }
        if (val <= 0.027344)
        {
            return Colour::fromFloatRGBA(0.0095610000000000001, 0.0077130000000000002, 0.055142999999999998, 1.0);
        }
        if (val <= 0.031250)
        {
            return Colour::fromFloatRGBA(0.011663, 0.009417, 0.063460000000000003, 1.0);
        }
        if (val <= 0.035156)
        {
            return Colour::fromFloatRGBA(0.013995, 0.011225000000000001, 0.071861999999999995, 1.0);
        }
        if (val <= 0.039062)
        {
            return Colour::fromFloatRGBA(0.016560999999999999, 0.013136, 0.080282000000000006, 1.0);
        }
        if (val <= 0.042969)
        {
            return Colour::fromFloatRGBA(0.019373000000000001, 0.015133000000000001, 0.088766999999999999, 1.0);
        }
        if (val <= 0.046875)
        {
            return Colour::fromFloatRGBA(0.022447000000000002, 0.017198999999999999, 0.097326999999999997, 1.0);
        }
        if (val <= 0.050781)
        {
            return Colour::fromFloatRGBA(0.025793, 0.019331000000000001, 0.10593, 1.0);
        }
        if (val <= 0.054688)
        {
            return Colour::fromFloatRGBA(0.029432, 0.021503000000000001, 0.114621, 1.0);
        }
        if (val <= 0.058594)
        {
            return Colour::fromFloatRGBA(0.033384999999999998, 0.023702000000000001, 0.12339700000000001, 1.0);
        }
        if (val <= 0.062500)
        {
            return Colour::fromFloatRGBA(0.037668, 0.025921, 0.13223199999999999, 1.0);
        }
        if (val <= 0.066406)
        {
            return Colour::fromFloatRGBA(0.042252999999999999, 0.028139000000000001, 0.14114099999999999, 1.0);
        }
        if (val <= 0.070312)
        {
            return Colour::fromFloatRGBA(0.046914999999999998, 0.030324, 0.15016399999999999, 1.0);
        }
        if (val <= 0.074219)
        {
            return Colour::fromFloatRGBA(0.051644000000000002, 0.032474000000000003, 0.15925400000000001, 1.0);
        }
        if (val <= 0.078125)
        {
            return Colour::fromFloatRGBA(0.056448999999999999, 0.034569000000000003, 0.16841400000000001, 1.0);
        }
        if (val <= 0.082031)
        {
            return Colour::fromFloatRGBA(0.061339999999999999, 0.036589999999999998, 0.17764199999999999, 1.0);
        }
        if (val <= 0.085938)
        {
            return Colour::fromFloatRGBA(0.066331000000000001, 0.038503999999999997, 0.18696199999999999, 1.0);
        }
        if (val <= 0.089844)
        {
            return Colour::fromFloatRGBA(0.071429000000000006, 0.040294000000000003, 0.196354, 1.0);
        }
        if (val <= 0.093750)
        {
            return Colour::fromFloatRGBA(0.076636999999999997, 0.041904999999999998, 0.20579900000000001, 1.0);
        }
        if (val <= 0.097656)
        {
            return Colour::fromFloatRGBA(0.081961999999999993, 0.043327999999999998, 0.21528900000000001, 1.0);
        }
        if (val <= 0.101562)
        {
            return Colour::fromFloatRGBA(0.087411000000000003, 0.044555999999999998, 0.22481300000000001, 1.0);
        }
        if (val <= 0.105469)
        {
            return Colour::fromFloatRGBA(0.092990000000000003, 0.045582999999999999, 0.23435800000000001, 1.0);
        }
        if (val <= 0.109375)
        {
            return Colour::fromFloatRGBA(0.098701999999999998, 0.046401999999999999, 0.24390400000000001, 1.0);
        }
        if (val <= 0.113281)
        {
            return Colour::fromFloatRGBA(0.10455100000000001, 0.047008000000000001, 0.25342999999999999, 1.0);
        }
        if (val <= 0.117188)
        {
            return Colour::fromFloatRGBA(0.110536, 0.047398999999999997, 0.26291199999999998, 1.0);
        }
        if (val <= 0.121094)
        {
            return Colour::fromFloatRGBA(0.116656, 0.047573999999999998, 0.27232099999999998, 1.0);
        }
        if (val <= 0.125000)
        {
            return Colour::fromFloatRGBA(0.122908, 0.047536000000000002, 0.28162399999999999, 1.0);
        }
        if (val <= 0.128906)
        {
            return Colour::fromFloatRGBA(0.12928500000000001, 0.047293000000000002, 0.29078799999999999, 1.0);
        }
        if (val <= 0.132812)
        {
            return Colour::fromFloatRGBA(0.13577800000000001, 0.046856000000000002, 0.29977599999999999, 1.0);
        }
        if (val <= 0.136719)
        {
            return Colour::fromFloatRGBA(0.142378, 0.046241999999999998, 0.30855300000000002, 1.0);
        }
        if (val <= 0.140625)
        {
            return Colour::fromFloatRGBA(0.14907300000000001, 0.045468000000000001, 0.31708500000000001, 1.0);
        }
        if (val <= 0.144531)
        {
            return Colour::fromFloatRGBA(0.15584999999999999, 0.044559000000000001, 0.32533800000000002, 1.0);
        }
        if (val <= 0.148438)
        {
            return Colour::fromFloatRGBA(0.162689, 0.043554000000000002, 0.33327699999999999, 1.0);
        }
        if (val <= 0.152344)
        {
            return Colour::fromFloatRGBA(0.169575, 0.042488999999999999, 0.34087400000000001, 1.0);
        }
        if (val <= 0.156250)
        {
            return Colour::fromFloatRGBA(0.17649300000000001, 0.041402000000000001, 0.348111, 1.0);
        }
        if (val <= 0.160156)
        {
            return Colour::fromFloatRGBA(0.18342900000000001, 0.040328999999999997, 0.35497099999999998, 1.0);
        }
        if (val <= 0.164062)
        {
            return Colour::fromFloatRGBA(0.19036700000000001, 0.039308999999999997, 0.36144700000000002, 1.0);
        }
        if (val <= 0.167969)
        {
            return Colour::fromFloatRGBA(0.197297, 0.038399999999999997, 0.367535, 1.0);
        }
        if (val <= 0.171875)
        {
            return Colour::fromFloatRGBA(0.204209, 0.037631999999999999, 0.37323800000000001, 1.0);
        }
        if (val <= 0.175781)
        {
            return Colour::fromFloatRGBA(0.211095, 0.03703, 0.37856299999999998, 1.0);
        }
        if (val <= 0.179688)
        {
            return Colour::fromFloatRGBA(0.217949, 0.036615000000000002, 0.38352199999999997, 1.0);
        }
        if (val <= 0.183594)
        {
            return Colour::fromFloatRGBA(0.22476299999999999, 0.036405, 0.388129, 1.0);
        }
        if (val <= 0.187500)
        {
            return Colour::fromFloatRGBA(0.23153799999999999, 0.036405, 0.39240000000000003, 1.0);
        }
        if (val <= 0.191406)
        {
            return Colour::fromFloatRGBA(0.23827300000000001, 0.036621000000000001, 0.39635300000000001, 1.0);
        }
        if (val <= 0.195312)
        {
            return Colour::fromFloatRGBA(0.24496699999999999, 0.037054999999999998, 0.400007, 1.0);
        }
        if (val <= 0.199219)
        {
            return Colour::fromFloatRGBA(0.25162000000000001, 0.037705000000000002, 0.40337800000000001, 1.0);
        }
        if (val <= 0.203125)
        {
            return Colour::fromFloatRGBA(0.25823400000000002, 0.038571000000000001, 0.40648499999999999, 1.0);
        }
        if (val <= 0.207031)
        {
            return Colour::fromFloatRGBA(0.26480999999999999, 0.039647000000000002, 0.40934500000000001, 1.0);
        }
        if (val <= 0.210938)
        {
            return Colour::fromFloatRGBA(0.271347, 0.040922, 0.41197600000000001, 1.0);
        }
        if (val <= 0.214844)
        {
            return Colour::fromFloatRGBA(0.27784999999999999, 0.042353000000000002, 0.41439199999999998, 1.0);
        }
        if (val <= 0.218750)
        {
            return Colour::fromFloatRGBA(0.28432099999999999, 0.043933, 0.41660799999999998, 1.0);
        }
        if (val <= 0.222656)
        {
            return Colour::fromFloatRGBA(0.29076299999999999, 0.045643999999999997, 0.41863699999999998, 1.0);
        }
        if (val <= 0.226562)
        {
            return Colour::fromFloatRGBA(0.297178, 0.047469999999999998, 0.420491, 1.0);
        }
        if (val <= 0.230469)
        {
            return Colour::fromFloatRGBA(0.303568, 0.049396000000000002, 0.422182, 1.0);
        }
        if (val <= 0.234375)
        {
            return Colour::fromFloatRGBA(0.30993500000000002, 0.051407000000000001, 0.42372100000000001, 1.0);
        }
        if (val <= 0.238281)
        {
            return Colour::fromFloatRGBA(0.31628200000000001, 0.053490000000000003, 0.42511599999999999, 1.0);
        }
        if (val <= 0.242188)
        {
            return Colour::fromFloatRGBA(0.32261000000000001, 0.055634000000000003, 0.42637700000000001, 1.0);
        }
        if (val <= 0.246094)
        {
            return Colour::fromFloatRGBA(0.32892100000000002, 0.057827000000000003, 0.42751099999999997, 1.0);
        }
        if (val <= 0.250000)
        {
            return Colour::fromFloatRGBA(0.33521699999999999, 0.060060000000000002, 0.42852400000000002, 1.0);
        }
        if (val <= 0.253906)
        {
            return Colour::fromFloatRGBA(0.34150000000000003, 0.062324999999999998, 0.429425, 1.0);
        }
        if (val <= 0.257812)
        {
            return Colour::fromFloatRGBA(0.347771, 0.064616000000000007, 0.43021700000000002, 1.0);
        }
        if (val <= 0.261719)
        {
            return Colour::fromFloatRGBA(0.35403200000000001, 0.066924999999999998, 0.43090600000000001, 1.0);
        }
        if (val <= 0.265625)
        {
            return Colour::fromFloatRGBA(0.36028399999999999, 0.069247000000000003, 0.43149700000000002, 1.0);
        }
        if (val <= 0.269531)
        {
            return Colour::fromFloatRGBA(0.36652899999999999, 0.071579000000000004, 0.43199399999999999, 1.0);
        }
        if (val <= 0.273438)
        {
            return Colour::fromFloatRGBA(0.37276799999999999, 0.073914999999999995, 0.43240000000000001, 1.0);
        }
        if (val <= 0.277344)
        {
            return Colour::fromFloatRGBA(0.37900099999999998, 0.076253000000000001, 0.43271900000000002, 1.0);
        }
        if (val <= 0.281250)
        {
            return Colour::fromFloatRGBA(0.38522800000000001, 0.078590999999999994, 0.43295499999999998, 1.0);
        }
        if (val <= 0.285156)
        {
            return Colour::fromFloatRGBA(0.391453, 0.080926999999999999, 0.43310900000000002, 1.0);
        }
        if (val <= 0.289062)
        {
            return Colour::fromFloatRGBA(0.39767400000000003, 0.083256999999999998, 0.43318299999999998, 1.0);
        }
        if (val <= 0.292969)
        {
            return Colour::fromFloatRGBA(0.40389399999999998, 0.085580000000000003, 0.43317899999999998, 1.0);
        }
        if (val <= 0.296875)
        {
            return Colour::fromFloatRGBA(0.41011300000000001, 0.087896000000000002, 0.43309799999999998, 1.0);
        }
        if (val <= 0.300781)
        {
            return Colour::fromFloatRGBA(0.41633100000000001, 0.090203000000000005, 0.43294300000000002, 1.0);
        }
        if (val <= 0.304688)
        {
            return Colour::fromFloatRGBA(0.42254900000000001, 0.092501, 0.43271399999999999, 1.0);
        }
        if (val <= 0.308594)
        {
            return Colour::fromFloatRGBA(0.42876799999999998, 0.094789999999999999, 0.43241200000000002, 1.0);
        }
        if (val <= 0.312500)
        {
            return Colour::fromFloatRGBA(0.43498700000000001, 0.097069000000000003, 0.43203900000000001, 1.0);
        }
        if (val <= 0.316406)
        {
            return Colour::fromFloatRGBA(0.44120700000000002, 0.099337999999999996, 0.43159399999999998, 1.0);
        }
        if (val <= 0.320312)
        {
            return Colour::fromFloatRGBA(0.44742799999999999, 0.10159700000000001, 0.43108000000000002, 1.0);
        }
        if (val <= 0.324219)
        {
            return Colour::fromFloatRGBA(0.45365100000000003, 0.103848, 0.43049799999999999, 1.0);
        }
        if (val <= 0.328125)
        {
            return Colour::fromFloatRGBA(0.45987499999999998, 0.106089, 0.42984600000000001, 1.0);
        }
        if (val <= 0.332031)
        {
            return Colour::fromFloatRGBA(0.46610000000000001, 0.108322, 0.42912499999999998, 1.0);
        }
        if (val <= 0.335938)
        {
            return Colour::fromFloatRGBA(0.47232800000000003, 0.11054700000000001, 0.42833399999999999, 1.0);
        }
        if (val <= 0.339844)
        {
            return Colour::fromFloatRGBA(0.47855799999999998, 0.112764, 0.42747499999999999, 1.0);
        }
        if (val <= 0.343750)
        {
            return Colour::fromFloatRGBA(0.48478900000000003, 0.11497400000000001, 0.42654799999999998, 1.0);
        }
        if (val <= 0.347656)
        {
            return Colour::fromFloatRGBA(0.49102200000000001, 0.11717900000000001, 0.42555199999999999, 1.0);
        }
        if (val <= 0.351562)
        {
            return Colour::fromFloatRGBA(0.497257, 0.119379, 0.42448799999999998, 1.0);
        }
        if (val <= 0.355469)
        {
            return Colour::fromFloatRGBA(0.50349299999999997, 0.121575, 0.42335600000000001, 1.0);
        }
        if (val <= 0.359375)
        {
            return Colour::fromFloatRGBA(0.50973000000000002, 0.123769, 0.42215599999999998, 1.0);
        }
        if (val <= 0.363281)
        {
            return Colour::fromFloatRGBA(0.51596699999999995, 0.12595999999999999, 0.42088700000000001, 1.0);
        }
        if (val <= 0.367188)
        {
            return Colour::fromFloatRGBA(0.52220599999999995, 0.12814999999999999, 0.41954900000000001, 1.0);
        }
        if (val <= 0.371094)
        {
            return Colour::fromFloatRGBA(0.52844400000000002, 0.13034100000000001, 0.41814200000000001, 1.0);
        }
        if (val <= 0.375000)
        {
            return Colour::fromFloatRGBA(0.53468300000000002, 0.13253400000000001, 0.41666700000000001, 1.0);
        }
        if (val <= 0.378906)
        {
            return Colour::fromFloatRGBA(0.54091999999999996, 0.13472899999999999, 0.41512300000000002, 1.0);
        }
        if (val <= 0.382812)
        {
            return Colour::fromFloatRGBA(0.547157, 0.136929, 0.41351100000000002, 1.0);
        }
        if (val <= 0.386719)
        {
            return Colour::fromFloatRGBA(0.553392, 0.13913400000000001, 0.411829, 1.0);
        }
        if (val <= 0.390625)
        {
            return Colour::fromFloatRGBA(0.55962400000000001, 0.141346, 0.410078, 1.0);
        }
        if (val <= 0.394531)
        {
            return Colour::fromFloatRGBA(0.56585399999999997, 0.143567, 0.40825800000000001, 1.0);
        }
        if (val <= 0.398438)
        {
            return Colour::fromFloatRGBA(0.57208099999999995, 0.14579700000000001, 0.40636899999999998, 1.0);
        }
        if (val <= 0.402344)
        {
            return Colour::fromFloatRGBA(0.57830400000000004, 0.148039, 0.40441100000000002, 1.0);
        }
        if (val <= 0.406250)
        {
            return Colour::fromFloatRGBA(0.58452099999999996, 0.15029400000000001, 0.40238499999999999, 1.0);
        }
        if (val <= 0.410156)
        {
            return Colour::fromFloatRGBA(0.59073399999999998, 0.152563, 0.40028999999999998, 1.0);
        }
        if (val <= 0.414062)
        {
            return Colour::fromFloatRGBA(0.59694000000000003, 0.15484800000000001, 0.39812500000000001, 1.0);
        }
        if (val <= 0.417969)
        {
            return Colour::fromFloatRGBA(0.60313899999999998, 0.15715100000000001, 0.39589099999999999, 1.0);
        }
        if (val <= 0.421875)
        {
            return Colour::fromFloatRGBA(0.60933000000000004, 0.159474, 0.39358900000000002, 1.0);
        }
        if (val <= 0.425781)
        {
            return Colour::fromFloatRGBA(0.61551299999999998, 0.16181699999999999, 0.39121899999999998, 1.0);
        }
        if (val <= 0.429688)
        {
            return Colour::fromFloatRGBA(0.62168500000000004, 0.164184, 0.38878099999999999, 1.0);
        }
        if (val <= 0.433594)
        {
            return Colour::fromFloatRGBA(0.62784700000000004, 0.166575, 0.38627600000000001, 1.0);
        }
        if (val <= 0.437500)
        {
            return Colour::fromFloatRGBA(0.63399799999999995, 0.168992, 0.38370399999999999, 1.0);
        }
        if (val <= 0.441406)
        {
            return Colour::fromFloatRGBA(0.64013500000000001, 0.17143800000000001, 0.38106499999999999, 1.0);
        }
        if (val <= 0.445312)
        {
            return Colour::fromFloatRGBA(0.64625999999999995, 0.17391400000000001, 0.378359, 1.0);
        }
        if (val <= 0.449219)
        {
            return Colour::fromFloatRGBA(0.65236899999999998, 0.17642099999999999, 0.37558599999999998, 1.0);
        }
        if (val <= 0.453125)
        {
            return Colour::fromFloatRGBA(0.65846300000000002, 0.17896200000000001, 0.37274800000000002, 1.0);
        }
        if (val <= 0.457031)
        {
            return Colour::fromFloatRGBA(0.66454000000000002, 0.18153900000000001, 0.36984600000000001, 1.0);
        }
        if (val <= 0.460938)
        {
            return Colour::fromFloatRGBA(0.67059899999999995, 0.18415300000000001, 0.36687900000000001, 1.0);
        }
        if (val <= 0.464844)
        {
            return Colour::fromFloatRGBA(0.67663799999999996, 0.186807, 0.36384899999999998, 1.0);
        }
        if (val <= 0.468750)
        {
            return Colour::fromFloatRGBA(0.68265600000000004, 0.189501, 0.36075699999999999, 1.0);
        }
        if (val <= 0.472656)
        {
            return Colour::fromFloatRGBA(0.68865299999999996, 0.19223899999999999, 0.357603, 1.0);
        }
        if (val <= 0.476562)
        {
            return Colour::fromFloatRGBA(0.69462699999999999, 0.195021, 0.35438799999999998, 1.0);
        }
        if (val <= 0.480469)
        {
            return Colour::fromFloatRGBA(0.70057599999999998, 0.197851, 0.35111300000000001, 1.0);
        }
        if (val <= 0.484375)
        {
            return Colour::fromFloatRGBA(0.70650000000000002, 0.20072799999999999, 0.347777, 1.0);
        }
        if (val <= 0.488281)
        {
            return Colour::fromFloatRGBA(0.71239600000000003, 0.203656, 0.34438299999999999, 1.0);
        }
        if (val <= 0.492188)
        {
            return Colour::fromFloatRGBA(0.71826400000000001, 0.20663599999999999, 0.34093099999999998, 1.0);
        }
        if (val <= 0.496094)
        {
            return Colour::fromFloatRGBA(0.72410300000000005, 0.20967, 0.337424, 1.0);
        }
        if (val <= 0.500000)
        {
            return Colour::fromFloatRGBA(0.72990900000000003, 0.212759, 0.33386100000000002, 1.0);
        }
        if (val <= 0.503906)
        {
            return Colour::fromFloatRGBA(0.73568299999999998, 0.21590599999999999, 0.33024500000000001, 1.0);
        }
        if (val <= 0.507812)
        {
            return Colour::fromFloatRGBA(0.74142300000000005, 0.219112, 0.32657599999999998, 1.0);
        }
        if (val <= 0.511719)
        {
            return Colour::fromFloatRGBA(0.74712699999999999, 0.22237799999999999, 0.32285599999999998, 1.0);
        }
        if (val <= 0.515625)
        {
            return Colour::fromFloatRGBA(0.75279399999999996, 0.22570599999999999, 0.31908500000000001, 1.0);
        }
        if (val <= 0.519531)
        {
            return Colour::fromFloatRGBA(0.75842200000000004, 0.229097, 0.31526599999999999, 1.0);
        }
        if (val <= 0.523438)
        {
            return Colour::fromFloatRGBA(0.76400999999999997, 0.23255400000000001, 0.31139899999999998, 1.0);
        }
        if (val <= 0.527344)
        {
            return Colour::fromFloatRGBA(0.76955600000000002, 0.23607700000000001, 0.30748500000000001, 1.0);
        }
        if (val <= 0.531250)
        {
            return Colour::fromFloatRGBA(0.77505900000000005, 0.23966699999999999, 0.30352600000000002, 1.0);
        }
        if (val <= 0.535156)
        {
            return Colour::fromFloatRGBA(0.78051700000000002, 0.24332699999999999, 0.29952299999999998, 1.0);
        }
        if (val <= 0.539062)
        {
            return Colour::fromFloatRGBA(0.78592899999999999, 0.247056, 0.29547699999999999, 1.0);
        }
        if (val <= 0.542969)
        {
            return Colour::fromFloatRGBA(0.79129300000000002, 0.25085600000000002, 0.29138999999999998, 1.0);
        }
        if (val <= 0.546875)
        {
            return Colour::fromFloatRGBA(0.79660699999999995, 0.25472800000000001, 0.28726400000000002, 1.0);
        }
        if (val <= 0.550781)
        {
            return Colour::fromFloatRGBA(0.801871, 0.25867400000000002, 0.28309899999999999, 1.0);
        }
        if (val <= 0.554688)
        {
            return Colour::fromFloatRGBA(0.80708199999999997, 0.26269199999999998, 0.27889799999999998, 1.0);
        }
        if (val <= 0.558594)
        {
            return Colour::fromFloatRGBA(0.81223900000000004, 0.26678600000000002, 0.27466099999999999, 1.0);
        }
        if (val <= 0.562500)
        {
            return Colour::fromFloatRGBA(0.81734099999999998, 0.27095399999999997, 0.27039000000000002, 1.0);
        }
        if (val <= 0.566406)
        {
            return Colour::fromFloatRGBA(0.82238599999999995, 0.27519700000000002, 0.26608500000000002, 1.0);
        }
        if (val <= 0.570312)
        {
            return Colour::fromFloatRGBA(0.827372, 0.27951700000000002, 0.26174999999999998, 1.0);
        }
        if (val <= 0.574219)
        {
            return Colour::fromFloatRGBA(0.83229900000000001, 0.28391300000000003, 0.25738299999999997, 1.0);
        }
        if (val <= 0.578125)
        {
            return Colour::fromFloatRGBA(0.83716500000000005, 0.288385, 0.25298799999999999, 1.0);
        }
        if (val <= 0.582031)
        {
            return Colour::fromFloatRGBA(0.84196899999999997, 0.292933, 0.24856400000000001, 1.0);
        }
        if (val <= 0.585938)
        {
            return Colour::fromFloatRGBA(0.84670900000000004, 0.29755900000000002, 0.244113, 1.0);
        }
        if (val <= 0.589844)
        {
            return Colour::fromFloatRGBA(0.85138400000000003, 0.30225999999999997, 0.23963599999999999, 1.0);
        }
        if (val <= 0.593750)
        {
            return Colour::fromFloatRGBA(0.85599199999999998, 0.30703799999999998, 0.23513300000000001, 1.0);
        }
        if (val <= 0.597656)
        {
            return Colour::fromFloatRGBA(0.86053299999999999, 0.311892, 0.23060600000000001, 1.0);
        }
        if (val <= 0.601562)
        {
            return Colour::fromFloatRGBA(0.86500600000000005, 0.31682199999999999, 0.22605500000000001, 1.0);
        }
        if (val <= 0.605469)
        {
            return Colour::fromFloatRGBA(0.86940899999999999, 0.32182699999999997, 0.22148200000000001, 1.0);
        }
        if (val <= 0.609375)
        {
            return Colour::fromFloatRGBA(0.87374099999999999, 0.32690599999999997, 0.216886, 1.0);
        }
        if (val <= 0.613281)
        {
            return Colour::fromFloatRGBA(0.87800100000000003, 0.33206000000000002, 0.21226800000000001, 1.0);
        }
        if (val <= 0.617188)
        {
            return Colour::fromFloatRGBA(0.88218799999999997, 0.337287, 0.20762800000000001, 1.0);
        }
        if (val <= 0.621094)
        {
            return Colour::fromFloatRGBA(0.88630200000000003, 0.342586, 0.20296800000000001, 1.0);
        }
        if (val <= 0.625000)
        {
            return Colour::fromFloatRGBA(0.89034100000000005, 0.34795700000000002, 0.19828599999999999, 1.0);
        }
        if (val <= 0.628906)
        {
            return Colour::fromFloatRGBA(0.89430500000000002, 0.35339900000000002, 0.19358400000000001, 1.0);
        }
        if (val <= 0.632812)
        {
            return Colour::fromFloatRGBA(0.89819199999999999, 0.35891099999999998, 0.18886, 1.0);
        }
        if (val <= 0.636719)
        {
            return Colour::fromFloatRGBA(0.902003, 0.36449199999999998, 0.184116, 1.0);
        }
        if (val <= 0.640625)
        {
            return Colour::fromFloatRGBA(0.90573499999999996, 0.37014000000000002, 0.17935000000000001, 1.0);
        }
        if (val <= 0.644531)
        {
            return Colour::fromFloatRGBA(0.90939000000000003, 0.37585600000000002, 0.174563, 1.0);
        }
        if (val <= 0.648438)
        {
            return Colour::fromFloatRGBA(0.91296600000000006, 0.38163599999999998, 0.16975499999999999, 1.0);
        }
        if (val <= 0.652344)
        {
            return Colour::fromFloatRGBA(0.916462, 0.38748100000000002, 0.16492399999999999, 1.0);
        }
        if (val <= 0.656250)
        {
            return Colour::fromFloatRGBA(0.919879, 0.39338899999999999, 0.16006999999999999, 1.0);
        }
        if (val <= 0.660156)
        {
            return Colour::fromFloatRGBA(0.92321500000000001, 0.39935900000000002, 0.155193, 1.0);
        }
        if (val <= 0.664062)
        {
            return Colour::fromFloatRGBA(0.92647000000000002, 0.405389, 0.15029200000000001, 1.0);
        }
        if (val <= 0.667969)
        {
            return Colour::fromFloatRGBA(0.92964400000000003, 0.41147899999999998, 0.145367, 1.0);
        }
        if (val <= 0.671875)
        {
            return Colour::fromFloatRGBA(0.93273700000000004, 0.41762700000000003, 0.14041699999999999, 1.0);
        }
        if (val <= 0.675781)
        {
            return Colour::fromFloatRGBA(0.935747, 0.42383100000000001, 0.13544, 1.0);
        }
        if (val <= 0.679688)
        {
            return Colour::fromFloatRGBA(0.93867500000000004, 0.430091, 0.130438, 1.0);
        }
        if (val <= 0.683594)
        {
            return Colour::fromFloatRGBA(0.94152100000000005, 0.43640499999999999, 0.12540899999999999, 1.0);
        }
        if (val <= 0.687500)
        {
            return Colour::fromFloatRGBA(0.94428500000000004, 0.442772, 0.120354, 1.0);
        }
        if (val <= 0.691406)
        {
            return Colour::fromFloatRGBA(0.94696499999999995, 0.44919100000000001, 0.115272, 1.0);
        }
        if (val <= 0.695312)
        {
            return Colour::fromFloatRGBA(0.94956200000000002, 0.45566000000000001, 0.110164, 1.0);
        }
        if (val <= 0.699219)
        {
            return Colour::fromFloatRGBA(0.952075, 0.46217799999999998, 0.105031, 1.0);
        }
        if (val <= 0.703125)
        {
            return Colour::fromFloatRGBA(0.95450599999999997, 0.46874399999999999, 0.099874000000000004, 1.0);
        }
        if (val <= 0.707031)
        {
            return Colour::fromFloatRGBA(0.95685200000000004, 0.475356, 0.094695000000000001, 1.0);
        }
        if (val <= 0.710938)
        {
            return Colour::fromFloatRGBA(0.95911400000000002, 0.482014, 0.089498999999999995, 1.0);
        }
        if (val <= 0.714844)
        {
            return Colour::fromFloatRGBA(0.96129299999999995, 0.48871599999999998, 0.084289000000000003, 1.0);
        }
        if (val <= 0.718750)
        {
            return Colour::fromFloatRGBA(0.96338699999999999, 0.49546200000000001, 0.079073000000000004, 1.0);
        }
        if (val <= 0.722656)
        {
            return Colour::fromFloatRGBA(0.96539699999999995, 0.50224899999999995, 0.073858999999999994, 1.0);
        }
        if (val <= 0.726562)
        {
            return Colour::fromFloatRGBA(0.96732200000000002, 0.50907800000000003, 0.068658999999999998, 1.0);
        }
        if (val <= 0.730469)
        {
            return Colour::fromFloatRGBA(0.969163, 0.51594600000000002, 0.063488000000000003, 1.0);
        }
        if (val <= 0.734375)
        {
            return Colour::fromFloatRGBA(0.97091899999999998, 0.52285300000000001, 0.058367000000000002, 1.0);
        }
        if (val <= 0.738281)
        {
            return Colour::fromFloatRGBA(0.97258999999999995, 0.52979799999999999, 0.053324000000000003, 1.0);
        }
        if (val <= 0.742188)
        {
            return Colour::fromFloatRGBA(0.97417600000000004, 0.53678000000000003, 0.048391999999999998, 1.0);
        }
        if (val <= 0.746094)
        {
            return Colour::fromFloatRGBA(0.97567700000000002, 0.543798, 0.043617999999999997, 1.0);
        }
        if (val <= 0.750000)
        {
            return Colour::fromFloatRGBA(0.97709199999999996, 0.55084999999999995, 0.039050000000000001, 1.0);
        }
        if (val <= 0.753906)
        {
            return Colour::fromFloatRGBA(0.97842200000000001, 0.55793700000000002, 0.034930999999999997, 1.0);
        }
        if (val <= 0.757812)
        {
            return Colour::fromFloatRGBA(0.97966600000000004, 0.56505700000000003, 0.031408999999999999, 1.0);
        }
        if (val <= 0.761719)
        {
            return Colour::fromFloatRGBA(0.98082400000000003, 0.57220899999999997, 0.028507999999999999, 1.0);
        }
        if (val <= 0.765625)
        {
            return Colour::fromFloatRGBA(0.98189499999999996, 0.57939200000000002, 0.026249999999999999, 1.0);
        }
        if (val <= 0.769531)
        {
            return Colour::fromFloatRGBA(0.982881, 0.58660599999999996, 0.024660999999999999, 1.0);
        }
        if (val <= 0.773438)
        {
            return Colour::fromFloatRGBA(0.98377899999999996, 0.59384899999999996, 0.023769999999999999, 1.0);
        }
        if (val <= 0.777344)
        {
            return Colour::fromFloatRGBA(0.98459099999999999, 0.60112200000000005, 0.023605999999999999, 1.0);
        }
        if (val <= 0.781250)
        {
            return Colour::fromFloatRGBA(0.98531500000000005, 0.60842200000000002, 0.024202000000000001, 1.0);
        }
        if (val <= 0.785156)
        {
            return Colour::fromFloatRGBA(0.98595200000000005, 0.61575000000000002, 0.025592, 1.0);
        }
        if (val <= 0.789062)
        {
            return Colour::fromFloatRGBA(0.98650199999999999, 0.62310500000000002, 0.027813999999999998, 1.0);
        }
        if (val <= 0.792969)
        {
            return Colour::fromFloatRGBA(0.98696399999999995, 0.63048499999999996, 0.030908000000000001, 1.0);
        }
        if (val <= 0.796875)
        {
            return Colour::fromFloatRGBA(0.98733700000000002, 0.63788999999999996, 0.034916000000000003, 1.0);
        }
        if (val <= 0.800781)
        {
            return Colour::fromFloatRGBA(0.987622, 0.64532, 0.039885999999999998, 1.0);
        }
        if (val <= 0.804688)
        {
            return Colour::fromFloatRGBA(0.987819, 0.65277300000000005, 0.045581000000000003, 1.0);
        }
        if (val <= 0.808594)
        {
            return Colour::fromFloatRGBA(0.98792599999999997, 0.66025, 0.051749999999999997, 1.0);
        }
        if (val <= 0.812500)
        {
            return Colour::fromFloatRGBA(0.98794499999999996, 0.66774800000000001, 0.058328999999999999, 1.0);
        }
        if (val <= 0.816406)
        {
            return Colour::fromFloatRGBA(0.98787400000000003, 0.67526699999999995, 0.065256999999999996, 1.0);
        }
        if (val <= 0.820312)
        {
            return Colour::fromFloatRGBA(0.98771399999999998, 0.68280700000000005, 0.072488999999999998, 1.0);
        }
        if (val <= 0.824219)
        {
            return Colour::fromFloatRGBA(0.98746400000000001, 0.69036600000000004, 0.079990000000000006, 1.0);
        }
        if (val <= 0.828125)
        {
            return Colour::fromFloatRGBA(0.987124, 0.69794400000000001, 0.087731000000000003, 1.0);
        }
        if (val <= 0.832031)
        {
            return Colour::fromFloatRGBA(0.98669399999999996, 0.70553999999999994, 0.095694000000000001, 1.0);
        }
        if (val <= 0.835938)
        {
            return Colour::fromFloatRGBA(0.98617500000000002, 0.71315300000000004, 0.103863, 1.0);
        }
        if (val <= 0.839844)
        {
            return Colour::fromFloatRGBA(0.98556600000000005, 0.72078200000000003, 0.112229, 1.0);
        }
        if (val <= 0.843750)
        {
            return Colour::fromFloatRGBA(0.98486499999999999, 0.72842700000000005, 0.120785, 1.0);
        }
        if (val <= 0.847656)
        {
            return Colour::fromFloatRGBA(0.98407500000000003, 0.73608700000000005, 0.129527, 1.0);
        }
        if (val <= 0.851562)
        {
            return Colour::fromFloatRGBA(0.98319599999999996, 0.74375800000000003, 0.13845299999999999, 1.0);
        }
        if (val <= 0.855469)
        {
            return Colour::fromFloatRGBA(0.98222799999999999, 0.75144200000000005, 0.147565, 1.0);
        }
        if (val <= 0.859375)
        {
            return Colour::fromFloatRGBA(0.98117299999999996, 0.759135, 0.156863, 1.0);
        }
        if (val <= 0.863281)
        {
            return Colour::fromFloatRGBA(0.98003200000000001, 0.76683699999999999, 0.166353, 1.0);
        }
        if (val <= 0.867188)
        {
            return Colour::fromFloatRGBA(0.97880599999999995, 0.77454500000000004, 0.176037, 1.0);
        }
        if (val <= 0.871094)
        {
            return Colour::fromFloatRGBA(0.97749699999999995, 0.78225800000000001, 0.185923, 1.0);
        }
        if (val <= 0.875000)
        {
            return Colour::fromFloatRGBA(0.97610799999999998, 0.78997399999999995, 0.196018, 1.0);
        }
        if (val <= 0.878906)
        {
            return Colour::fromFloatRGBA(0.974638, 0.79769199999999996, 0.20633199999999999, 1.0);
        }
        if (val <= 0.882812)
        {
            return Colour::fromFloatRGBA(0.97308799999999995, 0.80540900000000004, 0.21687699999999999, 1.0);
        }
        if (val <= 0.886719)
        {
            return Colour::fromFloatRGBA(0.971468, 0.81312200000000001, 0.227658, 1.0);
        }
        if (val <= 0.890625)
        {
            return Colour::fromFloatRGBA(0.96978299999999995, 0.82082500000000003, 0.23868600000000001, 1.0);
        }
        if (val <= 0.894531)
        {
            return Colour::fromFloatRGBA(0.96804100000000004, 0.828515, 0.249972, 1.0);
        }
        if (val <= 0.898438)
        {
            return Colour::fromFloatRGBA(0.96624299999999996, 0.83619100000000002, 0.26153399999999999, 1.0);
        }
        if (val <= 0.902344)
        {
            return Colour::fromFloatRGBA(0.96439399999999997, 0.84384800000000004, 0.273391, 1.0);
        }
        if (val <= 0.906250)
        {
            return Colour::fromFloatRGBA(0.96251699999999996, 0.85147600000000001, 0.28554600000000002, 1.0);
        }
        if (val <= 0.910156)
        {
            return Colour::fromFloatRGBA(0.96062599999999998, 0.85906899999999997, 0.29801, 1.0);
        }
        if (val <= 0.914062)
        {
            return Colour::fromFloatRGBA(0.95872000000000002, 0.86662399999999995, 0.31081999999999999, 1.0);
        }
        if (val <= 0.917969)
        {
            return Colour::fromFloatRGBA(0.95683399999999996, 0.87412900000000004, 0.32397399999999998, 1.0);
        }
        if (val <= 0.921875)
        {
            return Colour::fromFloatRGBA(0.95499699999999998, 0.88156900000000005, 0.33747500000000002, 1.0);
        }
        if (val <= 0.925781)
        {
            return Colour::fromFloatRGBA(0.95321500000000003, 0.88894200000000001, 0.35136899999999999, 1.0);
        }
        if (val <= 0.929688)
        {
            return Colour::fromFloatRGBA(0.951546, 0.89622599999999997, 0.36562699999999998, 1.0);
        }
        if (val <= 0.933594)
        {
            return Colour::fromFloatRGBA(0.95001800000000003, 0.90340900000000002, 0.38027100000000003, 1.0);
        }
        if (val <= 0.937500)
        {
            return Colour::fromFloatRGBA(0.94868300000000005, 0.91047299999999998, 0.395289, 1.0);
        }
        if (val <= 0.941406)
        {
            return Colour::fromFloatRGBA(0.94759400000000005, 0.91739899999999996, 0.410665, 1.0);
        }
        if (val <= 0.945312)
        {
            return Colour::fromFloatRGBA(0.94680900000000001, 0.92416799999999999, 0.426373, 1.0);
        }
        if (val <= 0.949219)
        {
            return Colour::fromFloatRGBA(0.94639200000000001, 0.93076099999999995, 0.44236700000000001, 1.0);
        }
        if (val <= 0.953125)
        {
            return Colour::fromFloatRGBA(0.94640299999999999, 0.93715899999999996, 0.458592, 1.0);
        }
        if (val <= 0.957031)
        {
            return Colour::fromFloatRGBA(0.94690300000000005, 0.94334799999999996, 0.47497, 1.0);
        }
        if (val <= 0.960938)
        {
            return Colour::fromFloatRGBA(0.94793700000000003, 0.949318, 0.49142599999999997, 1.0);
        }
        if (val <= 0.964844)
        {
            return Colour::fromFloatRGBA(0.94954499999999997, 0.955063, 0.50785999999999998, 1.0);
        }
        if (val <= 0.968750)
        {
            return Colour::fromFloatRGBA(0.95174000000000003, 0.96058699999999997, 0.52420299999999997, 1.0);
        }
        if (val <= 0.972656)
        {
            return Colour::fromFloatRGBA(0.95452899999999996, 0.96589599999999998, 0.54036099999999998, 1.0);
        }
        if (val <= 0.976562)
        {
            return Colour::fromFloatRGBA(0.95789599999999997, 0.97100299999999995, 0.55627499999999996, 1.0);
        }
        if (val <= 0.980469)
        {
            return Colour::fromFloatRGBA(0.961812, 0.97592400000000001, 0.57192500000000002, 1.0);
        }
        if (val <= 0.984375)
        {
            return Colour::fromFloatRGBA(0.96624900000000002, 0.98067800000000005, 0.58720600000000001, 1.0);
        }
        if (val <= 0.988281)
        {
            return Colour::fromFloatRGBA(0.97116199999999997, 0.98528199999999999, 0.60215399999999997, 1.0);
        }
        if (val <= 0.992188)
        {
            return Colour::fromFloatRGBA(0.97651100000000002, 0.98975299999999999, 0.61675999999999997, 1.0);
        }
        if (val <= 0.996094)
        {
            return Colour::fromFloatRGBA(0.98225700000000005, 0.99410900000000002, 0.63101700000000005, 1.0);
        }

        return Colour::fromFloatRGBA(0.98836199999999996, 0.99836400000000003, 0.64492400000000005, 1.0);
    }

    Colour colourFromMagma(float val)
    {
        if (val <= 0.003906)
        {
            return Colour::fromFloatRGBA(0.001462, 0.000466, 0.013866, 1.0);
        }
        if (val <= 0.007812)
        {
            return Colour::fromFloatRGBA(0.002258, 0.0012949999999999999, 0.018331, 1.0);
        }
        if (val <= 0.011719)
        {
            return Colour::fromFloatRGBA(0.0032789999999999998, 0.0023050000000000002, 0.023708, 1.0);
        }
        if (val <= 0.015625)
        {
            return Colour::fromFloatRGBA(0.0045120000000000004, 0.00349, 0.029964999999999999, 1.0);
        }
        if (val <= 0.019531)
        {
            return Colour::fromFloatRGBA(0.0059500000000000004, 0.0048430000000000001, 0.037130000000000003, 1.0);
        }
        if (val <= 0.023438)
        {
            return Colour::fromFloatRGBA(0.0075880000000000001, 0.0063559999999999997, 0.044972999999999999, 1.0);
        }
        if (val <= 0.027344)
        {
            return Colour::fromFloatRGBA(0.0094260000000000004, 0.0080219999999999996, 0.052844000000000002, 1.0);
        }
        if (val <= 0.031250)
        {
            return Colour::fromFloatRGBA(0.011464999999999999, 0.0098279999999999999, 0.060749999999999998, 1.0);
        }
        if (val <= 0.035156)
        {
            return Colour::fromFloatRGBA(0.013708, 0.011771, 0.068667000000000006, 1.0);
        }
        if (val <= 0.039062)
        {
            return Colour::fromFloatRGBA(0.016156, 0.01384, 0.076603000000000004, 1.0);
        }
        if (val <= 0.042969)
        {
            return Colour::fromFloatRGBA(0.018814999999999998, 0.016025999999999999, 0.084584000000000006, 1.0);
        }
        if (val <= 0.046875)
        {
            return Colour::fromFloatRGBA(0.021691999999999999, 0.018319999999999999, 0.092609999999999998, 1.0);
        }
        if (val <= 0.050781)
        {
            return Colour::fromFloatRGBA(0.024792000000000002, 0.020715000000000001, 0.100676, 1.0);
        }
        if (val <= 0.054688)
        {
            return Colour::fromFloatRGBA(0.028122999999999999, 0.023200999999999999, 0.10878699999999999, 1.0);
        }
        if (val <= 0.058594)
        {
            return Colour::fromFloatRGBA(0.031696000000000002, 0.025765, 0.116965, 1.0);
        }
        if (val <= 0.062500)
        {
            return Colour::fromFloatRGBA(0.035520000000000003, 0.028396999999999999, 0.12520899999999999, 1.0);
        }
        if (val <= 0.066406)
        {
            return Colour::fromFloatRGBA(0.039607999999999997, 0.03109, 0.13351499999999999, 1.0);
        }
        if (val <= 0.070312)
        {
            return Colour::fromFloatRGBA(0.043830000000000001, 0.033829999999999999, 0.14188600000000001, 1.0);
        }
        if (val <= 0.074219)
        {
            return Colour::fromFloatRGBA(0.048062000000000001, 0.036607000000000001, 0.15032699999999999, 1.0);
        }
        if (val <= 0.078125)
        {
            return Colour::fromFloatRGBA(0.052319999999999998, 0.039406999999999998, 0.15884100000000001, 1.0);
        }
        if (val <= 0.082031)
        {
            return Colour::fromFloatRGBA(0.056614999999999999, 0.042160000000000003, 0.16744600000000001, 1.0);
        }
        if (val <= 0.085938)
        {
            return Colour::fromFloatRGBA(0.060949000000000003, 0.044794, 0.17612900000000001, 1.0);
        }
        if (val <= 0.089844)
        {
            return Colour::fromFloatRGBA(0.065329999999999999, 0.047317999999999999, 0.184892, 1.0);
        }
        if (val <= 0.093750)
        {
            return Colour::fromFloatRGBA(0.069764000000000007, 0.049725999999999999, 0.19373499999999999, 1.0);
        }
        if (val <= 0.097656)
        {
            return Colour::fromFloatRGBA(0.074257000000000004, 0.052017000000000001, 0.20266000000000001, 1.0);
        }
        if (val <= 0.101562)
        {
            return Colour::fromFloatRGBA(0.078814999999999996, 0.054184000000000003, 0.21166699999999999, 1.0);
        }
        if (val <= 0.105469)
        {
            return Colour::fromFloatRGBA(0.083446000000000006, 0.056224999999999997, 0.22075500000000001, 1.0);
        }
        if (val <= 0.109375)
        {
            return Colour::fromFloatRGBA(0.088154999999999997, 0.058132999999999997, 0.22992199999999999, 1.0);
        }
        if (val <= 0.113281)
        {
            return Colour::fromFloatRGBA(0.092949000000000004, 0.059903999999999999, 0.23916399999999999, 1.0);
        }
        if (val <= 0.117188)
        {
            return Colour::fromFloatRGBA(0.097833000000000003, 0.061531000000000002, 0.248477, 1.0);
        }
        if (val <= 0.121094)
        {
            return Colour::fromFloatRGBA(0.102815, 0.063009999999999997, 0.25785400000000003, 1.0);
        }
        if (val <= 0.125000)
        {
            return Colour::fromFloatRGBA(0.10789899999999999, 0.064335000000000003, 0.267289, 1.0);
        }
        if (val <= 0.128906)
        {
            return Colour::fromFloatRGBA(0.113094, 0.065491999999999995, 0.27678399999999997, 1.0);
        }
        if (val <= 0.132812)
        {
            return Colour::fromFloatRGBA(0.118405, 0.066478999999999996, 0.28632099999999999, 1.0);
        }
        if (val <= 0.136719)
        {
            return Colour::fromFloatRGBA(0.123833, 0.067294999999999994, 0.295879, 1.0);
        }
        if (val <= 0.140625)
        {
            return Colour::fromFloatRGBA(0.12938, 0.067934999999999995, 0.30544300000000002, 1.0);
        }
        if (val <= 0.144531)
        {
            return Colour::fromFloatRGBA(0.13505300000000001, 0.068390999999999993, 0.315, 1.0);
        }
        if (val <= 0.148438)
        {
            return Colour::fromFloatRGBA(0.14085800000000001, 0.068654000000000007, 0.32453799999999999, 1.0);
        }
        if (val <= 0.152344)
        {
            return Colour::fromFloatRGBA(0.146785, 0.068737999999999994, 0.334011, 1.0);
        }
        if (val <= 0.156250)
        {
            return Colour::fromFloatRGBA(0.152839, 0.068637000000000004, 0.34340399999999999, 1.0);
        }
        if (val <= 0.160156)
        {
            return Colour::fromFloatRGBA(0.15901799999999999, 0.068353999999999998, 0.352688, 1.0);
        }
        if (val <= 0.164062)
        {
            return Colour::fromFloatRGBA(0.16530800000000001, 0.067910999999999999, 0.36181600000000003, 1.0);
        }
        if (val <= 0.167969)
        {
            return Colour::fromFloatRGBA(0.171713, 0.067305000000000004, 0.37077100000000002, 1.0);
        }
        if (val <= 0.171875)
        {
            return Colour::fromFloatRGBA(0.17821200000000001, 0.066575999999999996, 0.37949699999999997, 1.0);
        }
        if (val <= 0.175781)
        {
            return Colour::fromFloatRGBA(0.18480099999999999, 0.065731999999999999, 0.38797300000000001, 1.0);
        }
        if (val <= 0.179688)
        {
            return Colour::fromFloatRGBA(0.19145999999999999, 0.064818000000000001, 0.396152, 1.0);
        }
        if (val <= 0.183594)
        {
            return Colour::fromFloatRGBA(0.19817699999999999, 0.063862000000000002, 0.40400900000000001, 1.0);
        }
        if (val <= 0.187500)
        {
            return Colour::fromFloatRGBA(0.20493500000000001, 0.062907000000000005, 0.41151399999999999, 1.0);
        }
        if (val <= 0.191406)
        {
            return Colour::fromFloatRGBA(0.21171799999999999, 0.061991999999999998, 0.41864699999999999, 1.0);
        }
        if (val <= 0.195312)
        {
            return Colour::fromFloatRGBA(0.21851200000000001, 0.061157999999999997, 0.42539199999999999, 1.0);
        }
        if (val <= 0.199219)
        {
            return Colour::fromFloatRGBA(0.225302, 0.060444999999999999, 0.43174200000000001, 1.0);
        }
        if (val <= 0.203125)
        {
            return Colour::fromFloatRGBA(0.23207700000000001, 0.059888999999999998, 0.437695, 1.0);
        }
        if (val <= 0.207031)
        {
            return Colour::fromFloatRGBA(0.23882600000000001, 0.059517, 0.44325599999999998, 1.0);
        }
        if (val <= 0.210938)
        {
            return Colour::fromFloatRGBA(0.24554300000000001, 0.059352000000000002, 0.448436, 1.0);
        }
        if (val <= 0.214844)
        {
            return Colour::fromFloatRGBA(0.25222, 0.059415000000000003, 0.45324799999999998, 1.0);
        }
        if (val <= 0.218750)
        {
            return Colour::fromFloatRGBA(0.258857, 0.059706000000000002, 0.45771000000000001, 1.0);
        }
        if (val <= 0.222656)
        {
            return Colour::fromFloatRGBA(0.26544699999999999, 0.060236999999999999, 0.46183999999999997, 1.0);
        }
        if (val <= 0.226562)
        {
            return Colour::fromFloatRGBA(0.27199400000000001, 0.060994, 0.46566000000000002, 1.0);
        }
        if (val <= 0.230469)
        {
            return Colour::fromFloatRGBA(0.27849299999999999, 0.061977999999999998, 0.46919, 1.0);
        }
        if (val <= 0.234375)
        {
            return Colour::fromFloatRGBA(0.28495100000000001, 0.063168000000000002, 0.47245100000000001, 1.0);
        }
        if (val <= 0.238281)
        {
            return Colour::fromFloatRGBA(0.29136600000000001, 0.064552999999999999, 0.475462, 1.0);
        }
        if (val <= 0.242188)
        {
            return Colour::fromFloatRGBA(0.29774, 0.066116999999999995, 0.47824299999999997, 1.0);
        }
        if (val <= 0.246094)
        {
            return Colour::fromFloatRGBA(0.30408099999999999, 0.067835000000000006, 0.48081200000000002, 1.0);
        }
        if (val <= 0.250000)
        {
            return Colour::fromFloatRGBA(0.31038199999999999, 0.069702, 0.483186, 1.0);
        }
        if (val <= 0.253906)
        {
            return Colour::fromFloatRGBA(0.31665399999999999, 0.071690000000000004, 0.48537999999999998, 1.0);
        }
        if (val <= 0.257812)
        {
            return Colour::fromFloatRGBA(0.32289899999999999, 0.073782, 0.48740800000000001, 1.0);
        }
        if (val <= 0.261719)
        {
            return Colour::fromFloatRGBA(0.32911400000000002, 0.075971999999999998, 0.48928700000000003, 1.0);
        }
        if (val <= 0.265625)
        {
            return Colour::fromFloatRGBA(0.33530799999999999, 0.078236, 0.49102400000000002, 1.0);
        }
        if (val <= 0.269531)
        {
            return Colour::fromFloatRGBA(0.34148200000000001, 0.080563999999999997, 0.49263099999999999, 1.0);
        }
        if (val <= 0.273438)
        {
            return Colour::fromFloatRGBA(0.347636, 0.082946000000000006, 0.49412099999999998, 1.0);
        }
        if (val <= 0.277344)
        {
            return Colour::fromFloatRGBA(0.353773, 0.085373000000000004, 0.49550100000000002, 1.0);
        }
        if (val <= 0.281250)
        {
            return Colour::fromFloatRGBA(0.359898, 0.087831000000000006, 0.496778, 1.0);
        }
        if (val <= 0.285156)
        {
            return Colour::fromFloatRGBA(0.366012, 0.090314000000000005, 0.49796000000000001, 1.0);
        }
        if (val <= 0.289062)
        {
            return Colour::fromFloatRGBA(0.372116, 0.092815999999999996, 0.49905300000000002, 1.0);
        }
        if (val <= 0.292969)
        {
            return Colour::fromFloatRGBA(0.37821100000000002, 0.095332, 0.50006700000000004, 1.0);
        }
        if (val <= 0.296875)
        {
            return Colour::fromFloatRGBA(0.384299, 0.097854999999999998, 0.50100199999999995, 1.0);
        }
        if (val <= 0.300781)
        {
            return Colour::fromFloatRGBA(0.39038400000000001, 0.100379, 0.50186399999999998, 1.0);
        }
        if (val <= 0.304688)
        {
            return Colour::fromFloatRGBA(0.39646700000000001, 0.10290199999999999, 0.50265800000000005, 1.0);
        }
        if (val <= 0.308594)
        {
            return Colour::fromFloatRGBA(0.40254800000000002, 0.10542, 0.503386, 1.0);
        }
        if (val <= 0.312500)
        {
            return Colour::fromFloatRGBA(0.40862900000000002, 0.10793, 0.50405199999999994, 1.0);
        }
        if (val <= 0.316406)
        {
            return Colour::fromFloatRGBA(0.41470899999999999, 0.110431, 0.50466200000000005, 1.0);
        }
        if (val <= 0.320312)
        {
            return Colour::fromFloatRGBA(0.42079100000000003, 0.11292000000000001, 0.50521499999999997, 1.0);
        }
        if (val <= 0.324219)
        {
            return Colour::fromFloatRGBA(0.42687700000000001, 0.115395, 0.505714, 1.0);
        }
        if (val <= 0.328125)
        {
            return Colour::fromFloatRGBA(0.43296699999999999, 0.117855, 0.50616000000000005, 1.0);
        }
        if (val <= 0.332031)
        {
            return Colour::fromFloatRGBA(0.43906200000000001, 0.120298, 0.50655499999999998, 1.0);
        }
        if (val <= 0.335938)
        {
            return Colour::fromFloatRGBA(0.44516299999999998, 0.122724, 0.50690100000000005, 1.0);
        }
        if (val <= 0.339844)
        {
            return Colour::fromFloatRGBA(0.45127099999999998, 0.12513199999999999, 0.50719800000000004, 1.0);
        }
        if (val <= 0.343750)
        {
            return Colour::fromFloatRGBA(0.45738600000000001, 0.127522, 0.50744800000000001, 1.0);
        }
        if (val <= 0.347656)
        {
            return Colour::fromFloatRGBA(0.46350799999999998, 0.12989300000000001, 0.50765199999999999, 1.0);
        }
        if (val <= 0.351562)
        {
            return Colour::fromFloatRGBA(0.46964, 0.132245, 0.50780899999999995, 1.0);
        }
        if (val <= 0.355469)
        {
            return Colour::fromFloatRGBA(0.47577999999999998, 0.134577, 0.50792099999999996, 1.0);
        }
        if (val <= 0.359375)
        {
            return Colour::fromFloatRGBA(0.481929, 0.13689100000000001, 0.50798900000000002, 1.0);
        }
        if (val <= 0.363281)
        {
            return Colour::fromFloatRGBA(0.48808800000000002, 0.139186, 0.50801099999999999, 1.0);
        }
        if (val <= 0.367188)
        {
            return Colour::fromFloatRGBA(0.49425799999999998, 0.141462, 0.507988, 1.0);
        }
        if (val <= 0.371094)
        {
            return Colour::fromFloatRGBA(0.50043800000000005, 0.14371900000000001, 0.50792000000000004, 1.0);
        }
        if (val <= 0.375000)
        {
            return Colour::fromFloatRGBA(0.506629, 0.145958, 0.50780599999999998, 1.0);
        }
        if (val <= 0.378906)
        {
            return Colour::fromFloatRGBA(0.51283100000000004, 0.14817900000000001, 0.50764799999999999, 1.0);
        }
        if (val <= 0.382812)
        {
            return Colour::fromFloatRGBA(0.51904499999999998, 0.15038299999999999, 0.50744299999999998, 1.0);
        }
        if (val <= 0.386719)
        {
            return Colour::fromFloatRGBA(0.52527000000000001, 0.15256900000000001, 0.50719199999999998, 1.0);
        }
        if (val <= 0.390625)
        {
            return Colour::fromFloatRGBA(0.53150699999999995, 0.15473899999999999, 0.50689499999999998, 1.0);
        }
        if (val <= 0.394531)
        {
            return Colour::fromFloatRGBA(0.53775499999999998, 0.15689400000000001, 0.50655099999999997, 1.0);
        }
        if (val <= 0.398438)
        {
            return Colour::fromFloatRGBA(0.54401500000000003, 0.15903300000000001, 0.50615900000000003, 1.0);
        }
        if (val <= 0.402344)
        {
            return Colour::fromFloatRGBA(0.55028699999999997, 0.161158, 0.50571900000000003, 1.0);
        }
        if (val <= 0.406250)
        {
            return Colour::fromFloatRGBA(0.55657100000000004, 0.163269, 0.50522999999999996, 1.0);
        }
        if (val <= 0.410156)
        {
            return Colour::fromFloatRGBA(0.56286599999999998, 0.16536799999999999, 0.50469200000000003, 1.0);
        }
        if (val <= 0.414062)
        {
            return Colour::fromFloatRGBA(0.56917200000000001, 0.16745399999999999, 0.50410500000000003, 1.0);
        }
        if (val <= 0.417969)
        {
            return Colour::fromFloatRGBA(0.57548999999999995, 0.16952999999999999, 0.50346599999999997, 1.0);
        }
        if (val <= 0.421875)
        {
            return Colour::fromFloatRGBA(0.58181899999999998, 0.171596, 0.50277700000000003, 1.0);
        }
        if (val <= 0.425781)
        {
            return Colour::fromFloatRGBA(0.58815799999999996, 0.173652, 0.50203500000000001, 1.0);
        }
        if (val <= 0.429688)
        {
            return Colour::fromFloatRGBA(0.59450800000000004, 0.175701, 0.50124100000000005, 1.0);
        }
        if (val <= 0.433594)
        {
            return Colour::fromFloatRGBA(0.60086799999999996, 0.17774300000000001, 0.50039400000000001, 1.0);
        }
        if (val <= 0.437500)
        {
            return Colour::fromFloatRGBA(0.60723800000000006, 0.17977899999999999, 0.49949199999999999, 1.0);
        }
        if (val <= 0.441406)
        {
            return Colour::fromFloatRGBA(0.61361699999999997, 0.181811, 0.49853599999999998, 1.0);
        }
        if (val <= 0.445312)
        {
            return Colour::fromFloatRGBA(0.62000500000000003, 0.18384, 0.49752400000000002, 1.0);
        }
        if (val <= 0.449219)
        {
            return Colour::fromFloatRGBA(0.62640099999999999, 0.185867, 0.49645600000000001, 1.0);
        }
        if (val <= 0.453125)
        {
            return Colour::fromFloatRGBA(0.63280499999999995, 0.187893, 0.49533199999999999, 1.0);
        }
        if (val <= 0.457031)
        {
            return Colour::fromFloatRGBA(0.63921600000000001, 0.18992100000000001, 0.49414999999999998, 1.0);
        }
        if (val <= 0.460938)
        {
            return Colour::fromFloatRGBA(0.64563300000000001, 0.19195200000000001, 0.49291000000000001, 1.0);
        }
        if (val <= 0.464844)
        {
            return Colour::fromFloatRGBA(0.65205599999999997, 0.19398599999999999, 0.49161100000000002, 1.0);
        }
        if (val <= 0.468750)
        {
            return Colour::fromFloatRGBA(0.65848300000000004, 0.19602700000000001, 0.49025299999999999, 1.0);
        }
        if (val <= 0.472656)
        {
            return Colour::fromFloatRGBA(0.66491500000000003, 0.198075, 0.48883599999999999, 1.0);
        }
        if (val <= 0.476562)
        {
            return Colour::fromFloatRGBA(0.67134899999999997, 0.20013300000000001, 0.48735800000000001, 1.0);
        }
        if (val <= 0.480469)
        {
            return Colour::fromFloatRGBA(0.677786, 0.20220299999999999, 0.485819, 1.0);
        }
        if (val <= 0.484375)
        {
            return Colour::fromFloatRGBA(0.68422400000000005, 0.204286, 0.48421900000000001, 1.0);
        }
        if (val <= 0.488281)
        {
            return Colour::fromFloatRGBA(0.69066099999999997, 0.20638400000000001, 0.48255799999999999, 1.0);
        }
        if (val <= 0.492188)
        {
            return Colour::fromFloatRGBA(0.697098, 0.20850099999999999, 0.48083500000000001, 1.0);
        }
        if (val <= 0.496094)
        {
            return Colour::fromFloatRGBA(0.70353200000000005, 0.21063799999999999, 0.479049, 1.0);
        }
        if (val <= 0.500000)
        {
            return Colour::fromFloatRGBA(0.70996199999999998, 0.21279699999999999, 0.47720099999999999, 1.0);
        }
        if (val <= 0.503906)
        {
            return Colour::fromFloatRGBA(0.716387, 0.21498200000000001, 0.47528999999999999, 1.0);
        }
        if (val <= 0.507812)
        {
            return Colour::fromFloatRGBA(0.72280500000000003, 0.217194, 0.47331600000000001, 1.0);
        }
        if (val <= 0.511719)
        {
            return Colour::fromFloatRGBA(0.72921599999999998, 0.21943699999999999, 0.471279, 1.0);
        }
        if (val <= 0.515625)
        {
            return Colour::fromFloatRGBA(0.73561600000000005, 0.22171299999999999, 0.46917999999999999, 1.0);
        }
        if (val <= 0.519531)
        {
            return Colour::fromFloatRGBA(0.742004, 0.224025, 0.46701799999999999, 1.0);
        }
        if (val <= 0.523438)
        {
            return Colour::fromFloatRGBA(0.74837799999999999, 0.22637699999999999, 0.46479399999999998, 1.0);
        }
        if (val <= 0.527344)
        {
            return Colour::fromFloatRGBA(0.75473699999999999, 0.228772, 0.462509, 1.0);
        }
        if (val <= 0.531250)
        {
            return Colour::fromFloatRGBA(0.761077, 0.231214, 0.46016200000000002, 1.0);
        }
        if (val <= 0.535156)
        {
            return Colour::fromFloatRGBA(0.76739800000000002, 0.233705, 0.45775500000000002, 1.0);
        }
        if (val <= 0.539062)
        {
            return Colour::fromFloatRGBA(0.77369500000000002, 0.23624899999999999, 0.455289, 1.0);
        }
        if (val <= 0.542969)
        {
            return Colour::fromFloatRGBA(0.77996799999999999, 0.23885100000000001, 0.45276499999999997, 1.0);
        }
        if (val <= 0.546875)
        {
            return Colour::fromFloatRGBA(0.78621200000000002, 0.24151400000000001, 0.45018399999999997, 1.0);
        }
        if (val <= 0.550781)
        {
            return Colour::fromFloatRGBA(0.79242699999999999, 0.24424199999999999, 0.44754300000000002, 1.0);
        }
        if (val <= 0.554688)
        {
            return Colour::fromFloatRGBA(0.79860799999999998, 0.24704000000000001, 0.44484800000000002, 1.0);
        }
        if (val <= 0.558594)
        {
            return Colour::fromFloatRGBA(0.80475200000000002, 0.24991099999999999, 0.44210199999999999, 1.0);
        }
        if (val <= 0.562500)
        {
            return Colour::fromFloatRGBA(0.81085499999999999, 0.252861, 0.439305, 1.0);
        }
        if (val <= 0.566406)
        {
            return Colour::fromFloatRGBA(0.81691400000000003, 0.25589499999999998, 0.43646099999999999, 1.0);
        }
        if (val <= 0.570312)
        {
            return Colour::fromFloatRGBA(0.82292600000000005, 0.25901600000000002, 0.43357299999999999, 1.0);
        }
        if (val <= 0.574219)
        {
            return Colour::fromFloatRGBA(0.82888600000000001, 0.26222899999999999, 0.43064400000000003, 1.0);
        }
        if (val <= 0.578125)
        {
            return Colour::fromFloatRGBA(0.83479099999999995, 0.26554, 0.42767100000000002, 1.0);
        }
        if (val <= 0.582031)
        {
            return Colour::fromFloatRGBA(0.84063600000000005, 0.268953, 0.42466599999999999, 1.0);
        }
        if (val <= 0.585938)
        {
            return Colour::fromFloatRGBA(0.84641599999999995, 0.27247300000000002, 0.42163099999999998, 1.0);
        }
        if (val <= 0.589844)
        {
            return Colour::fromFloatRGBA(0.85212600000000005, 0.27610600000000002, 0.41857299999999997, 1.0);
        }
        if (val <= 0.593750)
        {
            return Colour::fromFloatRGBA(0.85776300000000005, 0.27985700000000002, 0.41549599999999998, 1.0);
        }
        if (val <= 0.597656)
        {
            return Colour::fromFloatRGBA(0.86331999999999998, 0.28372900000000001, 0.41240300000000002, 1.0);
        }
        if (val <= 0.601562)
        {
            return Colour::fromFloatRGBA(0.86879300000000004, 0.28772799999999998, 0.40930299999999997, 1.0);
        }
        if (val <= 0.605469)
        {
            return Colour::fromFloatRGBA(0.87417599999999995, 0.29185899999999998, 0.40620499999999998, 1.0);
        }
        if (val <= 0.609375)
        {
            return Colour::fromFloatRGBA(0.87946400000000002, 0.29612500000000003, 0.40311799999999998, 1.0);
        }
        if (val <= 0.613281)
        {
            return Colour::fromFloatRGBA(0.88465099999999997, 0.30053000000000002, 0.40004699999999999, 1.0);
        }
        if (val <= 0.617188)
        {
            return Colour::fromFloatRGBA(0.88973100000000005, 0.30507899999999999, 0.39700200000000002, 1.0);
        }
        if (val <= 0.621094)
        {
            return Colour::fromFloatRGBA(0.89470000000000005, 0.30977300000000002, 0.39399499999999998, 1.0);
        }
        if (val <= 0.625000)
        {
            return Colour::fromFloatRGBA(0.89955200000000002, 0.31461600000000001, 0.39103700000000002, 1.0);
        }
        if (val <= 0.628906)
        {
            return Colour::fromFloatRGBA(0.904281, 0.31961000000000001, 0.38813700000000001, 1.0);
        }
        if (val <= 0.632812)
        {
            return Colour::fromFloatRGBA(0.90888400000000003, 0.32475500000000002, 0.38530799999999998, 1.0);
        }
        if (val <= 0.636719)
        {
            return Colour::fromFloatRGBA(0.913354, 0.33005200000000001, 0.38256299999999999, 1.0);
        }
        if (val <= 0.640625)
        {
            return Colour::fromFloatRGBA(0.91768899999999998, 0.33550000000000002, 0.379915, 1.0);
        }
        if (val <= 0.644531)
        {
            return Colour::fromFloatRGBA(0.92188400000000004, 0.34109800000000001, 0.37737599999999999, 1.0);
        }
        if (val <= 0.648438)
        {
            return Colour::fromFloatRGBA(0.92593700000000001, 0.34684399999999999, 0.37495899999999999, 1.0);
        }
        if (val <= 0.652344)
        {
            return Colour::fromFloatRGBA(0.92984500000000003, 0.35273399999999999, 0.37267699999999998, 1.0);
        }
        if (val <= 0.656250)
        {
            return Colour::fromFloatRGBA(0.93360600000000005, 0.35876400000000003, 0.37054100000000001, 1.0);
        }
        if (val <= 0.660156)
        {
            return Colour::fromFloatRGBA(0.93722099999999997, 0.364929, 0.36856699999999998, 1.0);
        }
        if (val <= 0.664062)
        {
            return Colour::fromFloatRGBA(0.94068700000000005, 0.371224, 0.36676199999999998, 1.0);
        }
        if (val <= 0.667969)
        {
            return Colour::fromFloatRGBA(0.94400600000000001, 0.37764300000000001, 0.36513600000000002, 1.0);
        }
        if (val <= 0.671875)
        {
            return Colour::fromFloatRGBA(0.94718000000000002, 0.38417800000000002, 0.363701, 1.0);
        }
        if (val <= 0.675781)
        {
            return Colour::fromFloatRGBA(0.95021, 0.39082, 0.36246800000000001, 1.0);
        }
        if (val <= 0.679688)
        {
            return Colour::fromFloatRGBA(0.95309900000000003, 0.397563, 0.36143799999999998, 1.0);
        }
        if (val <= 0.683594)
        {
            return Colour::fromFloatRGBA(0.95584899999999995, 0.40439999999999998, 0.36061900000000002, 1.0);
        }
        if (val <= 0.687500)
        {
            return Colour::fromFloatRGBA(0.95846399999999998, 0.41132400000000002, 0.360014, 1.0);
        }
        if (val <= 0.691406)
        {
            return Colour::fromFloatRGBA(0.96094900000000005, 0.418323, 0.35963000000000001, 1.0);
        }
        if (val <= 0.695312)
        {
            return Colour::fromFloatRGBA(0.96331, 0.42538999999999999, 0.35946899999999998, 1.0);
        }
        if (val <= 0.699219)
        {
            return Colour::fromFloatRGBA(0.96554899999999999, 0.43251899999999999, 0.35952899999999999, 1.0);
        }
        if (val <= 0.703125)
        {
            return Colour::fromFloatRGBA(0.96767099999999995, 0.43970300000000001, 0.35981000000000002, 1.0);
        }
        if (val <= 0.707031)
        {
            return Colour::fromFloatRGBA(0.96967999999999999, 0.446936, 0.36031099999999999, 1.0);
        }
        if (val <= 0.710938)
        {
            return Colour::fromFloatRGBA(0.97158199999999995, 0.45421, 0.36103000000000002, 1.0);
        }
        if (val <= 0.714844)
        {
            return Colour::fromFloatRGBA(0.97338100000000005, 0.46151999999999999, 0.36196499999999998, 1.0);
        }
        if (val <= 0.718750)
        {
            return Colour::fromFloatRGBA(0.975082, 0.46886100000000003, 0.36311100000000002, 1.0);
        }
        if (val <= 0.722656)
        {
            return Colour::fromFloatRGBA(0.97668999999999995, 0.47622599999999998, 0.36446600000000001, 1.0);
        }
        if (val <= 0.726562)
        {
            return Colour::fromFloatRGBA(0.97821000000000002, 0.48361199999999999, 0.36602499999999999, 1.0);
        }
        if (val <= 0.730469)
        {
            return Colour::fromFloatRGBA(0.97964499999999999, 0.49101400000000001, 0.36778300000000003, 1.0);
        }
        if (val <= 0.734375)
        {
            return Colour::fromFloatRGBA(0.98099999999999998, 0.49842799999999998, 0.36973400000000001, 1.0);
        }
        if (val <= 0.738281)
        {
            return Colour::fromFloatRGBA(0.98227900000000001, 0.50585100000000005, 0.37187399999999998, 1.0);
        }
        if (val <= 0.742188)
        {
            return Colour::fromFloatRGBA(0.98348500000000005, 0.51327999999999996, 0.37419799999999998, 1.0);
        }
        if (val <= 0.746094)
        {
            return Colour::fromFloatRGBA(0.984622, 0.52071299999999998, 0.37669799999999998, 1.0);
        }
        if (val <= 0.750000)
        {
            return Colour::fromFloatRGBA(0.98569300000000004, 0.52814799999999995, 0.37937100000000001, 1.0);
        }
        if (val <= 0.753906)
        {
            return Colour::fromFloatRGBA(0.98670000000000002, 0.535582, 0.38220999999999999, 1.0);
        }
        if (val <= 0.757812)
        {
            return Colour::fromFloatRGBA(0.98764600000000002, 0.54301500000000003, 0.38521, 1.0);
        }
        if (val <= 0.761719)
        {
            return Colour::fromFloatRGBA(0.988533, 0.55044599999999999, 0.38836500000000002, 1.0);
        }
        if (val <= 0.765625)
        {
            return Colour::fromFloatRGBA(0.98936299999999999, 0.55787299999999995, 0.39167099999999999, 1.0);
        }
        if (val <= 0.769531)
        {
            return Colour::fromFloatRGBA(0.99013799999999996, 0.56529600000000002, 0.39512199999999997, 1.0);
        }
        if (val <= 0.773438)
        {
            return Colour::fromFloatRGBA(0.99087099999999995, 0.57270600000000005, 0.39871400000000001, 1.0);
        }
        if (val <= 0.777344)
        {
            return Colour::fromFloatRGBA(0.99155800000000005, 0.58010700000000004, 0.40244099999999999, 1.0);
        }
        if (val <= 0.781250)
        {
            return Colour::fromFloatRGBA(0.99219599999999997, 0.58750199999999997, 0.40629900000000002, 1.0);
        }
        if (val <= 0.785156)
        {
            return Colour::fromFloatRGBA(0.99278500000000003, 0.59489099999999995, 0.41028300000000001, 1.0);
        }
        if (val <= 0.789062)
        {
            return Colour::fromFloatRGBA(0.99332600000000004, 0.602275, 0.41438999999999998, 1.0);
        }
        if (val <= 0.792969)
        {
            return Colour::fromFloatRGBA(0.993834, 0.60964399999999996, 0.41861300000000001, 1.0);
        }
        if (val <= 0.796875)
        {
            return Colour::fromFloatRGBA(0.994309, 0.61699899999999996, 0.42294999999999999, 1.0);
        }
        if (val <= 0.800781)
        {
            return Colour::fromFloatRGBA(0.99473800000000001, 0.62434999999999996, 0.42739700000000003, 1.0);
        }
        if (val <= 0.804688)
        {
            return Colour::fromFloatRGBA(0.99512199999999995, 0.63169600000000004, 0.43195099999999997, 1.0);
        }
        if (val <= 0.808594)
        {
            return Colour::fromFloatRGBA(0.99548000000000003, 0.63902700000000001, 0.43660700000000002, 1.0);
        }
        if (val <= 0.812500)
        {
            return Colour::fromFloatRGBA(0.99580999999999997, 0.64634400000000003, 0.441361, 1.0);
        }
        if (val <= 0.816406)
        {
            return Colour::fromFloatRGBA(0.99609599999999998, 0.65365899999999999, 0.44621300000000003, 1.0);
        }
        if (val <= 0.820312)
        {
            return Colour::fromFloatRGBA(0.99634100000000003, 0.66096900000000003, 0.45116000000000001, 1.0);
        }
        if (val <= 0.824219)
        {
            return Colour::fromFloatRGBA(0.99658000000000002, 0.66825599999999996, 0.45619199999999999, 1.0);
        }
        if (val <= 0.828125)
        {
            return Colour::fromFloatRGBA(0.99677499999999997, 0.67554099999999995, 0.461314, 1.0);
        }
        if (val <= 0.832031)
        {
            return Colour::fromFloatRGBA(0.99692499999999995, 0.68282799999999999, 0.466526, 1.0);
        }
        if (val <= 0.835938)
        {
            return Colour::fromFloatRGBA(0.99707699999999999, 0.69008800000000003, 0.47181099999999998, 1.0);
        }
        if (val <= 0.839844)
        {
            return Colour::fromFloatRGBA(0.99718600000000002, 0.697349, 0.477182, 1.0);
        }
        if (val <= 0.843750)
        {
            return Colour::fromFloatRGBA(0.99725399999999997, 0.70461099999999999, 0.48263499999999998, 1.0);
        }
        if (val <= 0.847656)
        {
            return Colour::fromFloatRGBA(0.99732500000000002, 0.71184800000000004, 0.48815399999999998, 1.0);
        }
        if (val <= 0.851562)
        {
            return Colour::fromFloatRGBA(0.99735099999999999, 0.71908899999999998, 0.493755, 1.0);
        }
        if (val <= 0.855469)
        {
            return Colour::fromFloatRGBA(0.99735099999999999, 0.72632399999999997, 0.49942799999999998, 1.0);
        }
        if (val <= 0.859375)
        {
            return Colour::fromFloatRGBA(0.99734100000000003, 0.733545, 0.50516700000000003, 1.0);
        }
        if (val <= 0.863281)
        {
            return Colour::fromFloatRGBA(0.99728499999999998, 0.74077199999999999, 0.51098299999999997, 1.0);
        }
        if (val <= 0.867188)
        {
            return Colour::fromFloatRGBA(0.997228, 0.74798100000000001, 0.51685899999999996, 1.0);
        }
        if (val <= 0.871094)
        {
            return Colour::fromFloatRGBA(0.99713799999999997, 0.75519000000000003, 0.52280599999999999, 1.0);
        }
        if (val <= 0.875000)
        {
            return Colour::fromFloatRGBA(0.99701899999999999, 0.76239800000000002, 0.52882099999999999, 1.0);
        }
        if (val <= 0.878906)
        {
            return Colour::fromFloatRGBA(0.99689799999999995, 0.76959100000000003, 0.53489200000000003, 1.0);
        }
        if (val <= 0.882812)
        {
            return Colour::fromFloatRGBA(0.99672700000000003, 0.77679500000000001, 0.54103900000000005, 1.0);
        }
        if (val <= 0.886719)
        {
            return Colour::fromFloatRGBA(0.99657099999999998, 0.78397700000000003, 0.54723299999999997, 1.0);
        }
        if (val <= 0.890625)
        {
            return Colour::fromFloatRGBA(0.99636899999999995, 0.79116699999999995, 0.55349899999999996, 1.0);
        }
        if (val <= 0.894531)
        {
            return Colour::fromFloatRGBA(0.99616199999999999, 0.79834799999999995, 0.55981999999999998, 1.0);
        }
        if (val <= 0.898438)
        {
            return Colour::fromFloatRGBA(0.99593200000000004, 0.80552699999999999, 0.56620199999999998, 1.0);
        }
        if (val <= 0.902344)
        {
            return Colour::fromFloatRGBA(0.99568000000000001, 0.81270600000000004, 0.57264499999999996, 1.0);
        }
        if (val <= 0.906250)
        {
            return Colour::fromFloatRGBA(0.99542399999999998, 0.81987500000000002, 0.57913999999999999, 1.0);
        }
        if (val <= 0.910156)
        {
            return Colour::fromFloatRGBA(0.99513099999999999, 0.82705200000000001, 0.58570100000000003, 1.0);
        }
        if (val <= 0.914062)
        {
            return Colour::fromFloatRGBA(0.99485100000000004, 0.83421299999999998, 0.59230700000000003, 1.0);
        }
        if (val <= 0.917969)
        {
            return Colour::fromFloatRGBA(0.99452399999999996, 0.841387, 0.59898300000000004, 1.0);
        }
        if (val <= 0.921875)
        {
            return Colour::fromFloatRGBA(0.99422200000000005, 0.84853999999999996, 0.60569600000000001, 1.0);
        }
        if (val <= 0.925781)
        {
            return Colour::fromFloatRGBA(0.99386600000000003, 0.855711, 0.61248199999999997, 1.0);
        }
        if (val <= 0.929688)
        {
            return Colour::fromFloatRGBA(0.99354500000000001, 0.86285900000000004, 0.61929900000000004, 1.0);
        }
        if (val <= 0.933594)
        {
            return Colour::fromFloatRGBA(0.99317, 0.87002400000000002, 0.626189, 1.0);
        }
        if (val <= 0.937500)
        {
            return Colour::fromFloatRGBA(0.99283100000000002, 0.87716799999999995, 0.63310900000000003, 1.0);
        }
        if (val <= 0.941406)
        {
            return Colour::fromFloatRGBA(0.99243999999999999, 0.88432999999999995, 0.64009899999999997, 1.0);
        }
        if (val <= 0.945312)
        {
            return Colour::fromFloatRGBA(0.992089, 0.89146999999999998, 0.64711600000000002, 1.0);
        }
        if (val <= 0.949219)
        {
            return Colour::fromFloatRGBA(0.99168800000000001, 0.89862699999999995, 0.65420199999999995, 1.0);
        }
        if (val <= 0.953125)
        {
            return Colour::fromFloatRGBA(0.99133199999999999, 0.90576299999999998, 0.66130900000000004, 1.0);
        }
        if (val <= 0.957031)
        {
            return Colour::fromFloatRGBA(0.99092999999999998, 0.91291500000000003, 0.66848099999999999, 1.0);
        }
        if (val <= 0.960938)
        {
            return Colour::fromFloatRGBA(0.99056999999999995, 0.92004900000000001, 0.67567500000000003, 1.0);
        }
        if (val <= 0.964844)
        {
            return Colour::fromFloatRGBA(0.99017500000000003, 0.92719600000000002, 0.68292600000000003, 1.0);
        }
        if (val <= 0.968750)
        {
            return Colour::fromFloatRGBA(0.989815, 0.93432899999999997, 0.69019799999999998, 1.0);
        }
        if (val <= 0.972656)
        {
            return Colour::fromFloatRGBA(0.98943400000000004, 0.94147000000000003, 0.697519, 1.0);
        }
        if (val <= 0.976562)
        {
            return Colour::fromFloatRGBA(0.98907699999999998, 0.948604, 0.70486300000000002, 1.0);
        }
        if (val <= 0.980469)
        {
            return Colour::fromFloatRGBA(0.98871699999999996, 0.95574199999999998, 0.71224200000000004, 1.0);
        }
        if (val <= 0.984375)
        {
            return Colour::fromFloatRGBA(0.988367, 0.96287800000000001, 0.71964899999999998, 1.0);
        }
        if (val <= 0.988281)
        {
            return Colour::fromFloatRGBA(0.98803300000000005, 0.97001199999999999, 0.72707699999999997, 1.0);
        }
        if (val <= 0.992188)
        {
            return Colour::fromFloatRGBA(0.98769099999999999, 0.97715399999999997, 0.73453599999999997, 1.0);
        }
        if (val <= 0.996094)
        {
            return Colour::fromFloatRGBA(0.98738700000000001, 0.98428800000000005, 0.74200200000000005, 1.0);
        }

        return Colour::fromFloatRGBA(0.98705299999999996, 0.99143800000000004, 0.74950399999999995, 1.0);
    }

    Colour colourFromPlasma(float val)
    {
        if (val <= 0.003906)
        {
            return Colour::fromFloatRGBA(0.050382999999999997, 0.029803, 0.52797499999999997, 1.0);
        }
        if (val <= 0.007812)
        {
            return Colour::fromFloatRGBA(0.063535999999999995, 0.028426, 0.53312400000000004, 1.0);
        }
        if (val <= 0.011719)
        {
            return Colour::fromFloatRGBA(0.075353000000000003, 0.027206000000000001, 0.53800700000000001, 1.0);
        }
        if (val <= 0.015625)
        {
            return Colour::fromFloatRGBA(0.086221999999999993, 0.026124999999999999, 0.54265799999999997, 1.0);
        }
        if (val <= 0.019531)
        {
            return Colour::fromFloatRGBA(0.096379000000000006, 0.025165, 0.54710300000000001, 1.0);
        }
        if (val <= 0.023438)
        {
            return Colour::fromFloatRGBA(0.10598, 0.024309000000000001, 0.55136799999999997, 1.0);
        }
        if (val <= 0.027344)
        {
            return Colour::fromFloatRGBA(0.115124, 0.023556000000000001, 0.55546799999999996, 1.0);
        }
        if (val <= 0.031250)
        {
            return Colour::fromFloatRGBA(0.123903, 0.022877999999999999, 0.559423, 1.0);
        }
        if (val <= 0.035156)
        {
            return Colour::fromFloatRGBA(0.132381, 0.022258, 0.56325000000000003, 1.0);
        }
        if (val <= 0.039062)
        {
            return Colour::fromFloatRGBA(0.14060300000000001, 0.021687000000000001, 0.56695899999999999, 1.0);
        }
        if (val <= 0.042969)
        {
            return Colour::fromFloatRGBA(0.14860699999999999, 0.021153999999999999, 0.57056200000000001, 1.0);
        }
        if (val <= 0.046875)
        {
            return Colour::fromFloatRGBA(0.156421, 0.020650999999999999, 0.57406500000000005, 1.0);
        }
        if (val <= 0.050781)
        {
            return Colour::fromFloatRGBA(0.16406999999999999, 0.020171000000000001, 0.57747800000000005, 1.0);
        }
        if (val <= 0.054688)
        {
            return Colour::fromFloatRGBA(0.171574, 0.019706000000000001, 0.58080600000000004, 1.0);
        }
        if (val <= 0.058594)
        {
            return Colour::fromFloatRGBA(0.17895, 0.019251999999999998, 0.58405399999999996, 1.0);
        }
        if (val <= 0.062500)
        {
            return Colour::fromFloatRGBA(0.18621299999999999, 0.018803, 0.58722799999999997, 1.0);
        }
        if (val <= 0.066406)
        {
            return Colour::fromFloatRGBA(0.19337399999999999, 0.018353999999999999, 0.59033000000000002, 1.0);
        }
        if (val <= 0.070312)
        {
            return Colour::fromFloatRGBA(0.20044500000000001, 0.017902000000000001, 0.593364, 1.0);
        }
        if (val <= 0.074219)
        {
            return Colour::fromFloatRGBA(0.20743500000000001, 0.017441999999999999, 0.596333, 1.0);
        }
        if (val <= 0.078125)
        {
            return Colour::fromFloatRGBA(0.21435000000000001, 0.016972999999999999, 0.59923899999999997, 1.0);
        }
        if (val <= 0.082031)
        {
            return Colour::fromFloatRGBA(0.221197, 0.016497000000000001, 0.60208300000000003, 1.0);
        }
        if (val <= 0.085938)
        {
            return Colour::fromFloatRGBA(0.22798299999999999, 0.016007, 0.60486700000000004, 1.0);
        }
        if (val <= 0.089844)
        {
            return Colour::fromFloatRGBA(0.23471500000000001, 0.015502, 0.60759200000000002, 1.0);
        }
        if (val <= 0.093750)
        {
            return Colour::fromFloatRGBA(0.241396, 0.014978999999999999, 0.610259, 1.0);
        }
        if (val <= 0.097656)
        {
            return Colour::fromFloatRGBA(0.248032, 0.014439, 0.61286799999999997, 1.0);
        }
        if (val <= 0.101562)
        {
            return Colour::fromFloatRGBA(0.25462699999999999, 0.013882, 0.61541900000000005, 1.0);
        }
        if (val <= 0.105469)
        {
            return Colour::fromFloatRGBA(0.261183, 0.013308, 0.61791099999999999, 1.0);
        }
        if (val <= 0.109375)
        {
            return Colour::fromFloatRGBA(0.26770300000000002, 0.012716, 0.62034599999999995, 1.0);
        }
        if (val <= 0.113281)
        {
            return Colour::fromFloatRGBA(0.27419100000000002, 0.012109, 0.622722, 1.0);
        }
        if (val <= 0.117188)
        {
            return Colour::fromFloatRGBA(0.28064800000000001, 0.011488, 0.62503799999999998, 1.0);
        }
        if (val <= 0.121094)
        {
            return Colour::fromFloatRGBA(0.287076, 0.010855, 0.62729500000000005, 1.0);
        }
        if (val <= 0.125000)
        {
            return Colour::fromFloatRGBA(0.29347800000000002, 0.010213, 0.62948999999999999, 1.0);
        }
        if (val <= 0.128906)
        {
            return Colour::fromFloatRGBA(0.29985499999999998, 0.0095610000000000001, 0.63162399999999996, 1.0);
        }
        if (val <= 0.132812)
        {
            return Colour::fromFloatRGBA(0.30620999999999998, 0.0089020000000000002, 0.63369399999999998, 1.0);
        }
        if (val <= 0.136719)
        {
            return Colour::fromFloatRGBA(0.31254300000000002, 0.0082389999999999998, 0.63570000000000004, 1.0);
        }
        if (val <= 0.140625)
        {
            return Colour::fromFloatRGBA(0.31885599999999997, 0.0075760000000000003, 0.63763999999999998, 1.0);
        }
        if (val <= 0.144531)
        {
            return Colour::fromFloatRGBA(0.32514999999999999, 0.0069150000000000001, 0.63951199999999997, 1.0);
        }
        if (val <= 0.148438)
        {
            return Colour::fromFloatRGBA(0.331426, 0.0062610000000000001, 0.641316, 1.0);
        }
        if (val <= 0.152344)
        {
            return Colour::fromFloatRGBA(0.33768300000000001, 0.0056179999999999997, 0.64304899999999998, 1.0);
        }
        if (val <= 0.156250)
        {
            return Colour::fromFloatRGBA(0.34392499999999998, 0.0049909999999999998, 0.64471000000000001, 1.0);
        }
        if (val <= 0.160156)
        {
            return Colour::fromFloatRGBA(0.35015000000000002, 0.0043819999999999996, 0.64629800000000004, 1.0);
        }
        if (val <= 0.164062)
        {
            return Colour::fromFloatRGBA(0.35635899999999998, 0.0037980000000000002, 0.64781, 1.0);
        }
        if (val <= 0.167969)
        {
            return Colour::fromFloatRGBA(0.36255300000000001, 0.0032429999999999998, 0.64924499999999996, 1.0);
        }
        if (val <= 0.171875)
        {
            return Colour::fromFloatRGBA(0.36873299999999998, 0.0027239999999999999, 0.65060099999999998, 1.0);
        }
        if (val <= 0.175781)
        {
            return Colour::fromFloatRGBA(0.37489699999999998, 0.002245, 0.65187600000000001, 1.0);
        }
        if (val <= 0.179688)
        {
            return Colour::fromFloatRGBA(0.38104700000000002, 0.0018140000000000001, 0.65306799999999998, 1.0);
        }
        if (val <= 0.183594)
        {
            return Colour::fromFloatRGBA(0.387183, 0.0014339999999999999, 0.65417700000000001, 1.0);
        }
        if (val <= 0.187500)
        {
            return Colour::fromFloatRGBA(0.39330399999999999, 0.001114, 0.65519899999999998, 1.0);
        }
        if (val <= 0.191406)
        {
            return Colour::fromFloatRGBA(0.39941100000000002, 0.00085899999999999995, 0.65613299999999997, 1.0);
        }
        if (val <= 0.195312)
        {
            return Colour::fromFloatRGBA(0.405503, 0.000678, 0.65697700000000003, 1.0);
        }
        if (val <= 0.199219)
        {
            return Colour::fromFloatRGBA(0.41158, 0.00057700000000000004, 0.65773000000000004, 1.0);
        }
        if (val <= 0.203125)
        {
            return Colour::fromFloatRGBA(0.41764200000000001, 0.00056400000000000005, 0.65839000000000003, 1.0);
        }
        if (val <= 0.207031)
        {
            return Colour::fromFloatRGBA(0.42368899999999998, 0.00064599999999999998, 0.65895599999999999, 1.0);
        }
        if (val <= 0.210938)
        {
            return Colour::fromFloatRGBA(0.42971900000000002, 0.00083100000000000003, 0.65942500000000004, 1.0);
        }
        if (val <= 0.214844)
        {
            return Colour::fromFloatRGBA(0.43573400000000001, 0.001127, 0.65979699999999997, 1.0);
        }
        if (val <= 0.218750)
        {
            return Colour::fromFloatRGBA(0.44173200000000001, 0.0015399999999999999, 0.66006900000000002, 1.0);
        }
        if (val <= 0.222656)
        {
            return Colour::fromFloatRGBA(0.447714, 0.0020799999999999998, 0.66024000000000005, 1.0);
        }
        if (val <= 0.226562)
        {
            return Colour::fromFloatRGBA(0.453677, 0.0027550000000000001, 0.66030999999999995, 1.0);
        }
        if (val <= 0.230469)
        {
            return Colour::fromFloatRGBA(0.459623, 0.0035739999999999999, 0.660277, 1.0);
        }
        if (val <= 0.234375)
        {
            return Colour::fromFloatRGBA(0.46555000000000002, 0.0045450000000000004, 0.66013900000000003, 1.0);
        }
        if (val <= 0.238281)
        {
            return Colour::fromFloatRGBA(0.47145700000000001, 0.0056779999999999999, 0.65989699999999996, 1.0);
        }
        if (val <= 0.242188)
        {
            return Colour::fromFloatRGBA(0.47734399999999999, 0.0069800000000000001, 0.65954900000000005, 1.0);
        }
        if (val <= 0.246094)
        {
            return Colour::fromFloatRGBA(0.48320999999999997, 0.0084600000000000005, 0.65909499999999999, 1.0);
        }
        if (val <= 0.250000)
        {
            return Colour::fromFloatRGBA(0.48905500000000002, 0.010127000000000001, 0.65853399999999995, 1.0);
        }
        if (val <= 0.253906)
        {
            return Colour::fromFloatRGBA(0.49487700000000001, 0.011990000000000001, 0.65786500000000003, 1.0);
        }
        if (val <= 0.257812)
        {
            return Colour::fromFloatRGBA(0.50067799999999996, 0.014055, 0.65708800000000001, 1.0);
        }
        if (val <= 0.261719)
        {
            return Colour::fromFloatRGBA(0.50645399999999996, 0.016333, 0.65620199999999995, 1.0);
        }
        if (val <= 0.265625)
        {
            return Colour::fromFloatRGBA(0.51220600000000005, 0.018832999999999999, 0.65520900000000004, 1.0);
        }
        if (val <= 0.269531)
        {
            return Colour::fromFloatRGBA(0.51793299999999998, 0.021562999999999999, 0.65410900000000005, 1.0);
        }
        if (val <= 0.273438)
        {
            return Colour::fromFloatRGBA(0.52363300000000002, 0.024532000000000002, 0.65290099999999995, 1.0);
        }
        if (val <= 0.277344)
        {
            return Colour::fromFloatRGBA(0.52930600000000005, 0.027747000000000001, 0.651586, 1.0);
        }
        if (val <= 0.281250)
        {
            return Colour::fromFloatRGBA(0.53495199999999998, 0.031217000000000002, 0.65016499999999999, 1.0);
        }
        if (val <= 0.285156)
        {
            return Colour::fromFloatRGBA(0.54056999999999999, 0.034950000000000002, 0.64863999999999999, 1.0);
        }
        if (val <= 0.289062)
        {
            return Colour::fromFloatRGBA(0.546157, 0.038954000000000003, 0.64700999999999997, 1.0);
        }
        if (val <= 0.292969)
        {
            return Colour::fromFloatRGBA(0.55171499999999996, 0.043136000000000001, 0.64527699999999999, 1.0);
        }
        if (val <= 0.296875)
        {
            return Colour::fromFloatRGBA(0.55724300000000004, 0.047330999999999998, 0.64344299999999999, 1.0);
        }
        if (val <= 0.300781)
        {
            return Colour::fromFloatRGBA(0.56273799999999996, 0.051545000000000001, 0.641509, 1.0);
        }
        if (val <= 0.304688)
        {
            return Colour::fromFloatRGBA(0.56820099999999996, 0.055778000000000001, 0.63947699999999996, 1.0);
        }
        if (val <= 0.308594)
        {
            return Colour::fromFloatRGBA(0.57363200000000003, 0.060027999999999998, 0.63734900000000005, 1.0);
        }
        if (val <= 0.312500)
        {
            return Colour::fromFloatRGBA(0.57902900000000002, 0.064296000000000006, 0.63512599999999997, 1.0);
        }
        if (val <= 0.316406)
        {
            return Colour::fromFloatRGBA(0.58439099999999999, 0.068579000000000001, 0.63281200000000004, 1.0);
        }
        if (val <= 0.320312)
        {
            return Colour::fromFloatRGBA(0.58971899999999999, 0.072877999999999998, 0.63040799999999997, 1.0);
        }
        if (val <= 0.324219)
        {
            return Colour::fromFloatRGBA(0.59501099999999996, 0.077189999999999995, 0.62791699999999995, 1.0);
        }
        if (val <= 0.328125)
        {
            return Colour::fromFloatRGBA(0.60026599999999997, 0.081516000000000005, 0.62534199999999995, 1.0);
        }
        if (val <= 0.332031)
        {
            return Colour::fromFloatRGBA(0.60548500000000005, 0.085854, 0.62268599999999996, 1.0);
        }
        if (val <= 0.335938)
        {
            return Colour::fromFloatRGBA(0.61066699999999996, 0.090204000000000006, 0.61995100000000003, 1.0);
        }
        if (val <= 0.339844)
        {
            return Colour::fromFloatRGBA(0.61581200000000003, 0.094563999999999995, 0.61714000000000002, 1.0);
        }
        if (val <= 0.343750)
        {
            return Colour::fromFloatRGBA(0.620919, 0.098933999999999994, 0.61425700000000005, 1.0);
        }
        if (val <= 0.347656)
        {
            return Colour::fromFloatRGBA(0.62598699999999996, 0.103312, 0.61130499999999999, 1.0);
        }
        if (val <= 0.351562)
        {
            return Colour::fromFloatRGBA(0.63101700000000005, 0.107699, 0.60828700000000002, 1.0);
        }
        if (val <= 0.355469)
        {
            return Colour::fromFloatRGBA(0.63600800000000002, 0.112092, 0.60520499999999999, 1.0);
        }
        if (val <= 0.359375)
        {
            return Colour::fromFloatRGBA(0.64095899999999995, 0.116492, 0.60206499999999996, 1.0);
        }
        if (val <= 0.363281)
        {
            return Colour::fromFloatRGBA(0.645872, 0.12089800000000001, 0.59886700000000004, 1.0);
        }
        if (val <= 0.367188)
        {
            return Colour::fromFloatRGBA(0.65074600000000005, 0.125309, 0.59561699999999995, 1.0);
        }
        if (val <= 0.371094)
        {
            return Colour::fromFloatRGBA(0.65558000000000005, 0.12972500000000001, 0.59231699999999998, 1.0);
        }
        if (val <= 0.375000)
        {
            return Colour::fromFloatRGBA(0.66037400000000002, 0.13414400000000001, 0.58897100000000002, 1.0);
        }
        if (val <= 0.378906)
        {
            return Colour::fromFloatRGBA(0.66512899999999997, 0.13856599999999999, 0.58558200000000005, 1.0);
        }
        if (val <= 0.382812)
        {
            return Colour::fromFloatRGBA(0.66984500000000002, 0.14299200000000001, 0.58215399999999995, 1.0);
        }
        if (val <= 0.386719)
        {
            return Colour::fromFloatRGBA(0.67452199999999995, 0.14741899999999999, 0.57868799999999998, 1.0);
        }
        if (val <= 0.390625)
        {
            return Colour::fromFloatRGBA(0.67915999999999999, 0.15184800000000001, 0.57518899999999995, 1.0);
        }
        if (val <= 0.394531)
        {
            return Colour::fromFloatRGBA(0.68375799999999998, 0.156278, 0.57165999999999995, 1.0);
        }
        if (val <= 0.398438)
        {
            return Colour::fromFloatRGBA(0.68831799999999999, 0.16070899999999999, 0.56810300000000002, 1.0);
        }
        if (val <= 0.402344)
        {
            return Colour::fromFloatRGBA(0.69284000000000001, 0.16514100000000001, 0.56452199999999997, 1.0);
        }
        if (val <= 0.406250)
        {
            return Colour::fromFloatRGBA(0.69732400000000005, 0.169573, 0.56091899999999995, 1.0);
        }
        if (val <= 0.410156)
        {
            return Colour::fromFloatRGBA(0.70176899999999998, 0.17400499999999999, 0.55729600000000001, 1.0);
        }
        if (val <= 0.414062)
        {
            return Colour::fromFloatRGBA(0.70617799999999997, 0.17843700000000001, 0.55365699999999995, 1.0);
        }
        if (val <= 0.417969)
        {
            return Colour::fromFloatRGBA(0.71054899999999999, 0.182868, 0.55000400000000005, 1.0);
        }
        if (val <= 0.421875)
        {
            return Colour::fromFloatRGBA(0.71488300000000005, 0.18729899999999999, 0.54633799999999999, 1.0);
        }
        if (val <= 0.425781)
        {
            return Colour::fromFloatRGBA(0.71918099999999996, 0.19172900000000001, 0.54266300000000001, 1.0);
        }
        if (val <= 0.429688)
        {
            return Colour::fromFloatRGBA(0.72344399999999998, 0.196158, 0.53898100000000004, 1.0);
        }
        if (val <= 0.433594)
        {
            return Colour::fromFloatRGBA(0.72767000000000004, 0.20058599999999999, 0.53529300000000002, 1.0);
        }
        if (val <= 0.437500)
        {
            return Colour::fromFloatRGBA(0.73186200000000001, 0.205013, 0.53160099999999999, 1.0);
        }
        if (val <= 0.441406)
        {
            return Colour::fromFloatRGBA(0.73601899999999998, 0.20943899999999999, 0.52790800000000004, 1.0);
        }
        if (val <= 0.445312)
        {
            return Colour::fromFloatRGBA(0.740143, 0.213864, 0.52421600000000002, 1.0);
        }
        if (val <= 0.449219)
        {
            return Colour::fromFloatRGBA(0.744232, 0.21828800000000001, 0.52052399999999999, 1.0);
        }
        if (val <= 0.453125)
        {
            return Colour::fromFloatRGBA(0.74828899999999998, 0.22271099999999999, 0.51683400000000002, 1.0);
        }
        if (val <= 0.457031)
        {
            return Colour::fromFloatRGBA(0.75231199999999998, 0.227133, 0.51314899999999997, 1.0);
        }
        if (val <= 0.460938)
        {
            return Colour::fromFloatRGBA(0.75630399999999998, 0.23155500000000001, 0.50946800000000003, 1.0);
        }
        if (val <= 0.464844)
        {
            return Colour::fromFloatRGBA(0.76026400000000005, 0.23597599999999999, 0.50579399999999997, 1.0);
        }
        if (val <= 0.468750)
        {
            return Colour::fromFloatRGBA(0.76419300000000001, 0.240396, 0.50212599999999996, 1.0);
        }
        if (val <= 0.472656)
        {
            return Colour::fromFloatRGBA(0.76809000000000005, 0.24481700000000001, 0.49846499999999999, 1.0);
        }
        if (val <= 0.476562)
        {
            return Colour::fromFloatRGBA(0.77195800000000003, 0.24923699999999999, 0.494813, 1.0);
        }
        if (val <= 0.480469)
        {
            return Colour::fromFloatRGBA(0.77579600000000004, 0.25365799999999999, 0.49117100000000002, 1.0);
        }
        if (val <= 0.484375)
        {
            return Colour::fromFloatRGBA(0.77960399999999996, 0.25807799999999997, 0.487539, 1.0);
        }
        if (val <= 0.488281)
        {
            return Colour::fromFloatRGBA(0.78338300000000005, 0.26250000000000001, 0.48391800000000001, 1.0);
        }
        if (val <= 0.492188)
        {
            return Colour::fromFloatRGBA(0.78713299999999997, 0.26692199999999999, 0.48030699999999998, 1.0);
        }
        if (val <= 0.496094)
        {
            return Colour::fromFloatRGBA(0.79085499999999997, 0.271345, 0.47670600000000002, 1.0);
        }
        if (val <= 0.500000)
        {
            return Colour::fromFloatRGBA(0.79454899999999995, 0.27577000000000002, 0.47311700000000001, 1.0);
        }
        if (val <= 0.503906)
        {
            return Colour::fromFloatRGBA(0.79821600000000004, 0.28019699999999997, 0.46953800000000001, 1.0);
        }
        if (val <= 0.507812)
        {
            return Colour::fromFloatRGBA(0.80185499999999998, 0.28462599999999999, 0.46597100000000002, 1.0);
        }
        if (val <= 0.511719)
        {
            return Colour::fromFloatRGBA(0.80546700000000004, 0.28905700000000001, 0.46241500000000002, 1.0);
        }
        if (val <= 0.515625)
        {
            return Colour::fromFloatRGBA(0.80905199999999999, 0.293491, 0.45887, 1.0);
        }
        if (val <= 0.519531)
        {
            return Colour::fromFloatRGBA(0.812612, 0.29792800000000003, 0.45533800000000002, 1.0);
        }
        if (val <= 0.523438)
        {
            return Colour::fromFloatRGBA(0.81614399999999998, 0.30236800000000003, 0.451816, 1.0);
        }
        if (val <= 0.527344)
        {
            return Colour::fromFloatRGBA(0.81965100000000002, 0.30681199999999997, 0.44830599999999998, 1.0);
        }
        if (val <= 0.531250)
        {
            return Colour::fromFloatRGBA(0.82313199999999997, 0.31126100000000001, 0.44480599999999998, 1.0);
        }
        if (val <= 0.535156)
        {
            return Colour::fromFloatRGBA(0.82658799999999999, 0.31571399999999999, 0.44131599999999999, 1.0);
        }
        if (val <= 0.539062)
        {
            return Colour::fromFloatRGBA(0.83001800000000003, 0.32017200000000001, 0.437836, 1.0);
        }
        if (val <= 0.542969)
        {
            return Colour::fromFloatRGBA(0.833422, 0.32463500000000001, 0.43436599999999997, 1.0);
        }
        if (val <= 0.546875)
        {
            return Colour::fromFloatRGBA(0.83680100000000002, 0.32910499999999998, 0.43090499999999998, 1.0);
        }
        if (val <= 0.550781)
        {
            return Colour::fromFloatRGBA(0.84015499999999999, 0.33357999999999999, 0.42745499999999997, 1.0);
        }
        if (val <= 0.554688)
        {
            return Colour::fromFloatRGBA(0.84348400000000001, 0.33806199999999997, 0.42401299999999997, 1.0);
        }
        if (val <= 0.558594)
        {
            return Colour::fromFloatRGBA(0.84678799999999999, 0.34255099999999999, 0.42057899999999998, 1.0);
        }
        if (val <= 0.562500)
        {
            return Colour::fromFloatRGBA(0.85006599999999999, 0.34704800000000002, 0.417153, 1.0);
        }
        if (val <= 0.566406)
        {
            return Colour::fromFloatRGBA(0.85331900000000005, 0.351553, 0.41373399999999999, 1.0);
        }
        if (val <= 0.570312)
        {
            return Colour::fromFloatRGBA(0.85654699999999995, 0.35606599999999999, 0.41032200000000002, 1.0);
        }
        if (val <= 0.574219)
        {
            return Colour::fromFloatRGBA(0.85975000000000001, 0.36058800000000002, 0.40691699999999997, 1.0);
        }
        if (val <= 0.578125)
        {
            return Colour::fromFloatRGBA(0.862927, 0.36511900000000003, 0.40351900000000002, 1.0);
        }
        if (val <= 0.582031)
        {
            return Colour::fromFloatRGBA(0.86607800000000001, 0.36965999999999999, 0.40012599999999998, 1.0);
        }
        if (val <= 0.585938)
        {
            return Colour::fromFloatRGBA(0.86920299999999995, 0.37421199999999999, 0.39673799999999998, 1.0);
        }
        if (val <= 0.589844)
        {
            return Colour::fromFloatRGBA(0.87230300000000005, 0.378774, 0.39335500000000001, 1.0);
        }
        if (val <= 0.593750)
        {
            return Colour::fromFloatRGBA(0.87537600000000004, 0.38334699999999999, 0.38997599999999999, 1.0);
        }
        if (val <= 0.597656)
        {
            return Colour::fromFloatRGBA(0.87842299999999995, 0.387932, 0.3866, 1.0);
        }
        if (val <= 0.601562)
        {
            return Colour::fromFloatRGBA(0.88144299999999998, 0.39252900000000002, 0.38322899999999999, 1.0);
        }
        if (val <= 0.605469)
        {
            return Colour::fromFloatRGBA(0.884436, 0.39713900000000002, 0.37985999999999998, 1.0);
        }
        if (val <= 0.609375)
        {
            return Colour::fromFloatRGBA(0.88740200000000002, 0.40176200000000001, 0.376494, 1.0);
        }
        if (val <= 0.613281)
        {
            return Colour::fromFloatRGBA(0.89034000000000002, 0.40639799999999998, 0.37313000000000002, 1.0);
        }
        if (val <= 0.617188)
        {
            return Colour::fromFloatRGBA(0.89324999999999999, 0.41104800000000002, 0.36976799999999999, 1.0);
        }
        if (val <= 0.621094)
        {
            return Colour::fromFloatRGBA(0.89613100000000001, 0.41571200000000003, 0.36640699999999998, 1.0);
        }
        if (val <= 0.625000)
        {
            return Colour::fromFloatRGBA(0.89898400000000001, 0.42039199999999999, 0.36304700000000001, 1.0);
        }
        if (val <= 0.628906)
        {
            return Colour::fromFloatRGBA(0.90180700000000003, 0.42508699999999999, 0.35968800000000001, 1.0);
        }
        if (val <= 0.632812)
        {
            return Colour::fromFloatRGBA(0.90460099999999999, 0.42979699999999998, 0.35632900000000001, 1.0);
        }
        if (val <= 0.636719)
        {
            return Colour::fromFloatRGBA(0.90736499999999998, 0.43452400000000002, 0.35297000000000001, 1.0);
        }
        if (val <= 0.640625)
        {
            return Colour::fromFloatRGBA(0.91009799999999996, 0.43926799999999999, 0.34960999999999998, 1.0);
        }
        if (val <= 0.644531)
        {
            return Colour::fromFloatRGBA(0.91279999999999994, 0.44402900000000001, 0.34625099999999998, 1.0);
        }
        if (val <= 0.648438)
        {
            return Colour::fromFloatRGBA(0.91547100000000003, 0.44880700000000001, 0.34288999999999997, 1.0);
        }
        if (val <= 0.652344)
        {
            return Colour::fromFloatRGBA(0.91810899999999995, 0.45360299999999998, 0.33952900000000003, 1.0);
        }
        if (val <= 0.656250)
        {
            return Colour::fromFloatRGBA(0.92071400000000003, 0.45841700000000002, 0.33616600000000002, 1.0);
        }
        if (val <= 0.660156)
        {
            return Colour::fromFloatRGBA(0.92328699999999997, 0.46325100000000002, 0.33280100000000001, 1.0);
        }
        if (val <= 0.664062)
        {
            return Colour::fromFloatRGBA(0.92582500000000001, 0.46810299999999999, 0.32943499999999998, 1.0);
        }
        if (val <= 0.667969)
        {
            return Colour::fromFloatRGBA(0.92832899999999996, 0.47297499999999998, 0.326067, 1.0);
        }
        if (val <= 0.671875)
        {
            return Colour::fromFloatRGBA(0.93079800000000001, 0.47786699999999999, 0.32269700000000001, 1.0);
        }
        if (val <= 0.675781)
        {
            return Colour::fromFloatRGBA(0.93323199999999995, 0.48277999999999999, 0.31932500000000003, 1.0);
        }
        if (val <= 0.679688)
        {
            return Colour::fromFloatRGBA(0.93562999999999996, 0.48771199999999998, 0.31595200000000001, 1.0);
        }
        if (val <= 0.683594)
        {
            return Colour::fromFloatRGBA(0.93798999999999999, 0.49266700000000002, 0.31257499999999999, 1.0);
        }
        if (val <= 0.687500)
        {
            return Colour::fromFloatRGBA(0.94031299999999995, 0.49764199999999997, 0.309197, 1.0);
        }
        if (val <= 0.691406)
        {
            return Colour::fromFloatRGBA(0.94259800000000005, 0.50263899999999995, 0.30581599999999998, 1.0);
        }
        if (val <= 0.695312)
        {
            return Colour::fromFloatRGBA(0.94484400000000002, 0.50765800000000005, 0.30243300000000001, 1.0);
        }
        if (val <= 0.699219)
        {
            return Colour::fromFloatRGBA(0.94705099999999998, 0.51269900000000002, 0.29904900000000001, 1.0);
        }
        if (val <= 0.703125)
        {
            return Colour::fromFloatRGBA(0.94921699999999998, 0.51776299999999997, 0.29566199999999998, 1.0);
        }
        if (val <= 0.707031)
        {
            return Colour::fromFloatRGBA(0.95134399999999997, 0.52285000000000004, 0.29227500000000001, 1.0);
        }
        if (val <= 0.710938)
        {
            return Colour::fromFloatRGBA(0.95342800000000005, 0.52795999999999998, 0.288883, 1.0);
        }
        if (val <= 0.714844)
        {
            return Colour::fromFloatRGBA(0.95547000000000004, 0.53309300000000004, 0.28549000000000002, 1.0);
        }
        if (val <= 0.718750)
        {
            return Colour::fromFloatRGBA(0.95746900000000001, 0.53825000000000001, 0.28209600000000001, 1.0);
        }
        if (val <= 0.722656)
        {
            return Colour::fromFloatRGBA(0.95942400000000005, 0.543431, 0.27870099999999998, 1.0);
        }
        if (val <= 0.726562)
        {
            return Colour::fromFloatRGBA(0.96133599999999997, 0.54863600000000001, 0.27530500000000002, 1.0);
        }
        if (val <= 0.730469)
        {
            return Colour::fromFloatRGBA(0.96320300000000003, 0.55386500000000005, 0.27190900000000001, 1.0);
        }
        if (val <= 0.734375)
        {
            return Colour::fromFloatRGBA(0.96502399999999999, 0.559118, 0.268513, 1.0);
        }
        if (val <= 0.738281)
        {
            return Colour::fromFloatRGBA(0.96679800000000005, 0.56439600000000001, 0.26511800000000002, 1.0);
        }
        if (val <= 0.742188)
        {
            return Colour::fromFloatRGBA(0.968526, 0.56969999999999998, 0.26172099999999998, 1.0);
        }
        if (val <= 0.746094)
        {
            return Colour::fromFloatRGBA(0.97020499999999998, 0.57502799999999998, 0.25832500000000003, 1.0);
        }
        if (val <= 0.750000)
        {
            return Colour::fromFloatRGBA(0.971835, 0.58038199999999995, 0.25493100000000002, 1.0);
        }
        if (val <= 0.753906)
        {
            return Colour::fromFloatRGBA(0.97341599999999995, 0.58576099999999998, 0.25153999999999999, 1.0);
        }
        if (val <= 0.757812)
        {
            return Colour::fromFloatRGBA(0.97494700000000001, 0.59116500000000005, 0.24815100000000001, 1.0);
        }
        if (val <= 0.761719)
        {
            return Colour::fromFloatRGBA(0.97642799999999996, 0.59659499999999999, 0.24476700000000001, 1.0);
        }
        if (val <= 0.765625)
        {
            return Colour::fromFloatRGBA(0.97785599999999995, 0.602051, 0.24138699999999999, 1.0);
        }
        if (val <= 0.769531)
        {
            return Colour::fromFloatRGBA(0.97923300000000002, 0.60753199999999996, 0.238013, 1.0);
        }
        if (val <= 0.773438)
        {
            return Colour::fromFloatRGBA(0.98055599999999998, 0.613039, 0.23464599999999999, 1.0);
        }
        if (val <= 0.777344)
        {
            return Colour::fromFloatRGBA(0.98182599999999998, 0.61857200000000001, 0.23128699999999999, 1.0);
        }
        if (val <= 0.781250)
        {
            return Colour::fromFloatRGBA(0.98304100000000005, 0.62413099999999999, 0.227937, 1.0);
        }
        if (val <= 0.785156)
        {
            return Colour::fromFloatRGBA(0.98419900000000005, 0.629718, 0.22459499999999999, 1.0);
        }
        if (val <= 0.789062)
        {
            return Colour::fromFloatRGBA(0.98530099999999998, 0.63532999999999995, 0.22126499999999999, 1.0);
        }
        if (val <= 0.792969)
        {
            return Colour::fromFloatRGBA(0.98634500000000003, 0.64096900000000001, 0.217948, 1.0);
        }
        if (val <= 0.796875)
        {
            return Colour::fromFloatRGBA(0.98733199999999999, 0.64663300000000001, 0.21464800000000001, 1.0);
        }
        if (val <= 0.800781)
        {
            return Colour::fromFloatRGBA(0.98826000000000003, 0.65232500000000004, 0.211364, 1.0);
        }
        if (val <= 0.804688)
        {
            return Colour::fromFloatRGBA(0.98912800000000001, 0.65804300000000004, 0.20810000000000001, 1.0);
        }
        if (val <= 0.808594)
        {
            return Colour::fromFloatRGBA(0.98993500000000001, 0.66378700000000002, 0.20485900000000001, 1.0);
        }
        if (val <= 0.812500)
        {
            return Colour::fromFloatRGBA(0.99068100000000003, 0.66955799999999999, 0.20164199999999999, 1.0);
        }
        if (val <= 0.816406)
        {
            return Colour::fromFloatRGBA(0.99136500000000005, 0.67535500000000004, 0.19845299999999999, 1.0);
        }
        if (val <= 0.820312)
        {
            return Colour::fromFloatRGBA(0.99198500000000001, 0.68117899999999998, 0.195295, 1.0);
        }
        if (val <= 0.824219)
        {
            return Colour::fromFloatRGBA(0.99254100000000001, 0.68703000000000003, 0.19217000000000001, 1.0);
        }
        if (val <= 0.828125)
        {
            return Colour::fromFloatRGBA(0.99303200000000003, 0.69290700000000005, 0.189084, 1.0);
        }
        if (val <= 0.832031)
        {
            return Colour::fromFloatRGBA(0.99345600000000001, 0.69881000000000004, 0.18604100000000001, 1.0);
        }
        if (val <= 0.835938)
        {
            return Colour::fromFloatRGBA(0.99381399999999998, 0.70474099999999995, 0.18304300000000001, 1.0);
        }
        if (val <= 0.839844)
        {
            return Colour::fromFloatRGBA(0.99410299999999996, 0.71069800000000005, 0.18009700000000001, 1.0);
        }
        if (val <= 0.843750)
        {
            return Colour::fromFloatRGBA(0.99432399999999999, 0.71668100000000001, 0.177208, 1.0);
        }
        if (val <= 0.847656)
        {
            return Colour::fromFloatRGBA(0.99447399999999997, 0.72269099999999997, 0.17438100000000001, 1.0);
        }
        if (val <= 0.851562)
        {
            return Colour::fromFloatRGBA(0.99455300000000002, 0.72872800000000004, 0.171622, 1.0);
        }
        if (val <= 0.855469)
        {
            return Colour::fromFloatRGBA(0.99456100000000003, 0.73479099999999997, 0.168938, 1.0);
        }
        if (val <= 0.859375)
        {
            return Colour::fromFloatRGBA(0.99449500000000002, 0.74087999999999998, 0.16633500000000001, 1.0);
        }
        if (val <= 0.863281)
        {
            return Colour::fromFloatRGBA(0.99435499999999999, 0.74699499999999996, 0.16382099999999999, 1.0);
        }
        if (val <= 0.867188)
        {
            return Colour::fromFloatRGBA(0.99414100000000005, 0.75313699999999995, 0.16140399999999999, 1.0);
        }
        if (val <= 0.871094)
        {
            return Colour::fromFloatRGBA(0.99385100000000004, 0.75930399999999998, 0.15909200000000001, 1.0);
        }
        if (val <= 0.875000)
        {
            return Colour::fromFloatRGBA(0.99348199999999998, 0.76549900000000004, 0.156891, 1.0);
        }
        if (val <= 0.878906)
        {
            return Colour::fromFloatRGBA(0.99303300000000005, 0.77171999999999996, 0.154808, 1.0);
        }
        if (val <= 0.882812)
        {
            return Colour::fromFloatRGBA(0.99250499999999997, 0.77796699999999996, 0.15285499999999999, 1.0);
        }
        if (val <= 0.886719)
        {
            return Colour::fromFloatRGBA(0.99189700000000003, 0.78423900000000002, 0.15104200000000001, 1.0);
        }
        if (val <= 0.890625)
        {
            return Colour::fromFloatRGBA(0.99120900000000001, 0.79053700000000005, 0.14937700000000001, 1.0);
        }
        if (val <= 0.894531)
        {
            return Colour::fromFloatRGBA(0.99043899999999996, 0.79685899999999998, 0.14787, 1.0);
        }
        if (val <= 0.898438)
        {
            return Colour::fromFloatRGBA(0.98958699999999999, 0.80320499999999995, 0.14652899999999999, 1.0);
        }
        if (val <= 0.902344)
        {
            return Colour::fromFloatRGBA(0.98864799999999997, 0.80957900000000005, 0.14535699999999999, 1.0);
        }
        if (val <= 0.906250)
        {
            return Colour::fromFloatRGBA(0.98762099999999997, 0.81597799999999998, 0.14436299999999999, 1.0);
        }
        if (val <= 0.910156)
        {
            return Colour::fromFloatRGBA(0.98650899999999997, 0.82240100000000005, 0.14355699999999999, 1.0);
        }
        if (val <= 0.914062)
        {
            return Colour::fromFloatRGBA(0.98531400000000002, 0.82884599999999997, 0.14294499999999999, 1.0);
        }
        if (val <= 0.917969)
        {
            return Colour::fromFloatRGBA(0.98403099999999999, 0.83531500000000003, 0.14252799999999999, 1.0);
        }
        if (val <= 0.921875)
        {
            return Colour::fromFloatRGBA(0.982653, 0.841812, 0.14230300000000001, 1.0);
        }
        if (val <= 0.925781)
        {
            return Colour::fromFloatRGBA(0.98119000000000001, 0.848329, 0.14227899999999999, 1.0);
        }
        if (val <= 0.929688)
        {
            return Colour::fromFloatRGBA(0.97964399999999996, 0.85486600000000001, 0.142453, 1.0);
        }
        if (val <= 0.933594)
        {
            return Colour::fromFloatRGBA(0.97799499999999995, 0.86143199999999998, 0.14280799999999999, 1.0);
        }
        if (val <= 0.937500)
        {
            return Colour::fromFloatRGBA(0.97626500000000005, 0.86801600000000001, 0.14335100000000001, 1.0);
        }
        if (val <= 0.941406)
        {
            return Colour::fromFloatRGBA(0.97444299999999995, 0.87462200000000001, 0.14406099999999999, 1.0);
        }
        if (val <= 0.945312)
        {
            return Colour::fromFloatRGBA(0.97253000000000001, 0.88124999999999998, 0.144923, 1.0);
        }
        if (val <= 0.949219)
        {
            return Colour::fromFloatRGBA(0.97053299999999998, 0.88789600000000002, 0.14591899999999999, 1.0);
        }
        if (val <= 0.953125)
        {
            return Colour::fromFloatRGBA(0.96844300000000005, 0.89456400000000003, 0.14701400000000001, 1.0);
        }
        if (val <= 0.957031)
        {
            return Colour::fromFloatRGBA(0.96627099999999999, 0.90124899999999997, 0.14818000000000001, 1.0);
        }
        if (val <= 0.960938)
        {
            return Colour::fromFloatRGBA(0.96402100000000002, 0.90795000000000003, 0.14937, 1.0);
        }
        if (val <= 0.964844)
        {
            return Colour::fromFloatRGBA(0.96168100000000001, 0.91467200000000004, 0.15051999999999999, 1.0);
        }
        if (val <= 0.968750)
        {
            return Colour::fromFloatRGBA(0.95927600000000002, 0.92140699999999998, 0.15156600000000001, 1.0);
        }
        if (val <= 0.972656)
        {
            return Colour::fromFloatRGBA(0.95680799999999999, 0.92815199999999998, 0.15240899999999999, 1.0);
        }
        if (val <= 0.976562)
        {
            return Colour::fromFloatRGBA(0.954287, 0.93490799999999996, 0.152921, 1.0);
        }
        if (val <= 0.980469)
        {
            return Colour::fromFloatRGBA(0.95172599999999996, 0.94167100000000004, 0.15292500000000001, 1.0);
        }
        if (val <= 0.984375)
        {
            return Colour::fromFloatRGBA(0.94915099999999997, 0.94843500000000003, 0.15217800000000001, 1.0);
        }
        if (val <= 0.988281)
        {
            return Colour::fromFloatRGBA(0.94660200000000005, 0.95518999999999998, 0.15032799999999999, 1.0);
        }
        if (val <= 0.992188)
        {
            return Colour::fromFloatRGBA(0.94415199999999999, 0.96191599999999999, 0.14686099999999999, 1.0);
        }
        if (val <= 0.996094)
        {
            return Colour::fromFloatRGBA(0.94189599999999996, 0.96858999999999995, 0.140956, 1.0);
        }

        return Colour::fromFloatRGBA(0.94001500000000004, 0.97515799999999997, 0.131326, 1.0);
    }

    Colour colourFromViridis(float val)
    {
        if (val <= 0.003906)
        {
            return Colour::fromFloatRGBA(0.26700400000000002, 0.0048739999999999999, 0.32941500000000001, 1.0);
        }
        if (val <= 0.007812)
        {
            return Colour::fromFloatRGBA(0.26851000000000003, 0.0096050000000000007, 0.33542699999999998, 1.0);
        }
        if (val <= 0.011719)
        {
            return Colour::fromFloatRGBA(0.26994400000000002, 0.014625000000000001, 0.34137899999999999, 1.0);
        }
        if (val <= 0.015625)
        {
            return Colour::fromFloatRGBA(0.27130500000000002, 0.019942000000000001, 0.34726899999999999, 1.0);
        }
        if (val <= 0.019531)
        {
            return Colour::fromFloatRGBA(0.272594, 0.025562999999999999, 0.35309299999999999, 1.0);
        }
        if (val <= 0.023438)
        {
            return Colour::fromFloatRGBA(0.27380900000000002, 0.031496999999999997, 0.35885299999999998, 1.0);
        }
        if (val <= 0.027344)
        {
            return Colour::fromFloatRGBA(0.27495199999999997, 0.037752000000000001, 0.36454300000000001, 1.0);
        }
        if (val <= 0.031250)
        {
            return Colour::fromFloatRGBA(0.27602199999999999, 0.044166999999999998, 0.37016399999999999, 1.0);
        }
        if (val <= 0.035156)
        {
            return Colour::fromFloatRGBA(0.27701799999999999, 0.050344, 0.37571500000000002, 1.0);
        }
        if (val <= 0.039062)
        {
            return Colour::fromFloatRGBA(0.27794099999999999, 0.056323999999999999, 0.381191, 1.0);
        }
        if (val <= 0.042969)
        {
            return Colour::fromFloatRGBA(0.27879100000000001, 0.062144999999999999, 0.38659199999999999, 1.0);
        }
        if (val <= 0.046875)
        {
            return Colour::fromFloatRGBA(0.27956599999999998, 0.067835999999999994, 0.39191700000000002, 1.0);
        }
        if (val <= 0.050781)
        {
            return Colour::fromFloatRGBA(0.28026699999999999, 0.073416999999999996, 0.39716299999999999, 1.0);
        }
        if (val <= 0.054688)
        {
            return Colour::fromFloatRGBA(0.28089399999999998, 0.078907000000000005, 0.40232899999999999, 1.0);
        }
        if (val <= 0.058594)
        {
            return Colour::fromFloatRGBA(0.28144599999999997, 0.084320000000000006, 0.407414, 1.0);
        }
        if (val <= 0.062500)
        {
            return Colour::fromFloatRGBA(0.28192400000000001, 0.089665999999999996, 0.41241499999999998, 1.0);
        }
        if (val <= 0.066406)
        {
            return Colour::fromFloatRGBA(0.28232699999999999, 0.094954999999999998, 0.41733100000000001, 1.0);
        }
        if (val <= 0.070312)
        {
            return Colour::fromFloatRGBA(0.28265600000000002, 0.10019599999999999, 0.42215999999999998, 1.0);
        }
        if (val <= 0.074219)
        {
            return Colour::fromFloatRGBA(0.28290999999999999, 0.105393, 0.426902, 1.0);
        }
        if (val <= 0.078125)
        {
            return Colour::fromFloatRGBA(0.28309099999999998, 0.110553, 0.43155399999999999, 1.0);
        }
        if (val <= 0.082031)
        {
            return Colour::fromFloatRGBA(0.28319699999999998, 0.11568000000000001, 0.43611499999999997, 1.0);
        }
        if (val <= 0.085938)
        {
            return Colour::fromFloatRGBA(0.28322900000000001, 0.120777, 0.44058399999999998, 1.0);
        }
        if (val <= 0.089844)
        {
            return Colour::fromFloatRGBA(0.28318700000000002, 0.12584799999999999, 0.44496000000000002, 1.0);
        }
        if (val <= 0.093750)
        {
            return Colour::fromFloatRGBA(0.28307199999999999, 0.13089500000000001, 0.449241, 1.0);
        }
        if (val <= 0.097656)
        {
            return Colour::fromFloatRGBA(0.28288400000000002, 0.13592000000000001, 0.45342700000000002, 1.0);
        }
        if (val <= 0.101562)
        {
            return Colour::fromFloatRGBA(0.28262300000000001, 0.140926, 0.45751700000000001, 1.0);
        }
        if (val <= 0.105469)
        {
            return Colour::fromFloatRGBA(0.28228999999999999, 0.14591199999999999, 0.46150999999999998, 1.0);
        }
        if (val <= 0.109375)
        {
            return Colour::fromFloatRGBA(0.281887, 0.15088099999999999, 0.46540500000000001, 1.0);
        }
        if (val <= 0.113281)
        {
            return Colour::fromFloatRGBA(0.281412, 0.155834, 0.46920099999999998, 1.0);
        }
        if (val <= 0.117188)
        {
            return Colour::fromFloatRGBA(0.28086800000000001, 0.160771, 0.47289900000000001, 1.0);
        }
        if (val <= 0.121094)
        {
            return Colour::fromFloatRGBA(0.28025499999999998, 0.16569300000000001, 0.47649799999999998, 1.0);
        }
        if (val <= 0.125000)
        {
            return Colour::fromFloatRGBA(0.27957399999999999, 0.170599, 0.47999700000000001, 1.0);
        }
        if (val <= 0.128906)
        {
            return Colour::fromFloatRGBA(0.27882600000000002, 0.17549000000000001, 0.48339700000000002, 1.0);
        }
        if (val <= 0.132812)
        {
            return Colour::fromFloatRGBA(0.27801199999999998, 0.180367, 0.48669699999999999, 1.0);
        }
        if (val <= 0.136719)
        {
            return Colour::fromFloatRGBA(0.27713399999999999, 0.185228, 0.489898, 1.0);
        }
        if (val <= 0.140625)
        {
            return Colour::fromFloatRGBA(0.276194, 0.19007399999999999, 0.49300100000000002, 1.0);
        }
        if (val <= 0.144531)
        {
            return Colour::fromFloatRGBA(0.27519100000000002, 0.19490499999999999, 0.49600499999999997, 1.0);
        }
        if (val <= 0.148438)
        {
            return Colour::fromFloatRGBA(0.27412799999999998, 0.19972100000000001, 0.49891099999999999, 1.0);
        }
        if (val <= 0.152344)
        {
            return Colour::fromFloatRGBA(0.27300600000000003, 0.20452000000000001, 0.50172099999999997, 1.0);
        }
        if (val <= 0.156250)
        {
            return Colour::fromFloatRGBA(0.27182800000000001, 0.20930299999999999, 0.50443400000000005, 1.0);
        }
        if (val <= 0.160156)
        {
            return Colour::fromFloatRGBA(0.27059499999999997, 0.21406900000000001, 0.50705199999999995, 1.0);
        }
        if (val <= 0.164062)
        {
            return Colour::fromFloatRGBA(0.26930799999999999, 0.21881800000000001, 0.50957699999999995, 1.0);
        }
        if (val <= 0.167969)
        {
            return Colour::fromFloatRGBA(0.26796799999999998, 0.223549, 0.51200800000000002, 1.0);
        }
        if (val <= 0.171875)
        {
            return Colour::fromFloatRGBA(0.26657999999999998, 0.22826199999999999, 0.51434899999999995, 1.0);
        }
        if (val <= 0.175781)
        {
            return Colour::fromFloatRGBA(0.26514500000000002, 0.232956, 0.51659900000000003, 1.0);
        }
        if (val <= 0.179688)
        {
            return Colour::fromFloatRGBA(0.26366299999999998, 0.23763100000000001, 0.51876199999999995, 1.0);
        }
        if (val <= 0.183594)
        {
            return Colour::fromFloatRGBA(0.26213799999999998, 0.242286, 0.52083699999999999, 1.0);
        }
        if (val <= 0.187500)
        {
            return Colour::fromFloatRGBA(0.260571, 0.246922, 0.52282799999999996, 1.0);
        }
        if (val <= 0.191406)
        {
            return Colour::fromFloatRGBA(0.258965, 0.25153700000000001, 0.52473599999999998, 1.0);
        }
        if (val <= 0.195312)
        {
            return Colour::fromFloatRGBA(0.257322, 0.25613000000000002, 0.526563, 1.0);
        }
        if (val <= 0.199219)
        {
            return Colour::fromFloatRGBA(0.25564500000000001, 0.26070300000000002, 0.528312, 1.0);
        }
        if (val <= 0.203125)
        {
            return Colour::fromFloatRGBA(0.25393500000000002, 0.26525399999999999, 0.52998299999999998, 1.0);
        }
        if (val <= 0.207031)
        {
            return Colour::fromFloatRGBA(0.25219399999999997, 0.269783, 0.53157900000000002, 1.0);
        }
        if (val <= 0.210938)
        {
            return Colour::fromFloatRGBA(0.25042500000000001, 0.27428999999999998, 0.53310299999999999, 1.0);
        }
        if (val <= 0.214844)
        {
            return Colour::fromFloatRGBA(0.24862899999999999, 0.278775, 0.53455600000000003, 1.0);
        }
        if (val <= 0.218750)
        {
            return Colour::fromFloatRGBA(0.246811, 0.28323700000000002, 0.535941, 1.0);
        }
        if (val <= 0.222656)
        {
            return Colour::fromFloatRGBA(0.244972, 0.28767500000000001, 0.53725999999999996, 1.0);
        }
        if (val <= 0.226562)
        {
            return Colour::fromFloatRGBA(0.243113, 0.29209200000000002, 0.53851599999999999, 1.0);
        }
        if (val <= 0.230469)
        {
            return Colour::fromFloatRGBA(0.24123700000000001, 0.296485, 0.53970899999999999, 1.0);
        }
        if (val <= 0.234375)
        {
            return Colour::fromFloatRGBA(0.239346, 0.30085499999999998, 0.54084399999999999, 1.0);
        }
        if (val <= 0.238281)
        {
            return Colour::fromFloatRGBA(0.23744100000000001, 0.30520199999999997, 0.54192099999999999, 1.0);
        }
        if (val <= 0.242188)
        {
            return Colour::fromFloatRGBA(0.23552600000000001, 0.309527, 0.54294399999999998, 1.0);
        }
        if (val <= 0.246094)
        {
            return Colour::fromFloatRGBA(0.23360300000000001, 0.313828, 0.54391400000000001, 1.0);
        }
        if (val <= 0.250000)
        {
            return Colour::fromFloatRGBA(0.23167399999999999, 0.318106, 0.54483400000000004, 1.0);
        }
        if (val <= 0.253906)
        {
            return Colour::fromFloatRGBA(0.229739, 0.32236100000000001, 0.54570600000000002, 1.0);
        }
        if (val <= 0.257812)
        {
            return Colour::fromFloatRGBA(0.227802, 0.326594, 0.54653200000000002, 1.0);
        }
        if (val <= 0.261719)
        {
            return Colour::fromFloatRGBA(0.22586300000000001, 0.33080500000000002, 0.54731399999999997, 1.0);
        }
        if (val <= 0.265625)
        {
            return Colour::fromFloatRGBA(0.22392500000000001, 0.33499400000000001, 0.54805300000000001, 1.0);
        }
        if (val <= 0.269531)
        {
            return Colour::fromFloatRGBA(0.22198899999999999, 0.33916099999999999, 0.54875200000000002, 1.0);
        }
        if (val <= 0.273438)
        {
            return Colour::fromFloatRGBA(0.220057, 0.34330699999999997, 0.54941300000000004, 1.0);
        }
        if (val <= 0.277344)
        {
            return Colour::fromFloatRGBA(0.21812999999999999, 0.34743200000000002, 0.55003800000000003, 1.0);
        }
        if (val <= 0.281250)
        {
            return Colour::fromFloatRGBA(0.21621000000000001, 0.35153499999999999, 0.55062699999999998, 1.0);
        }
        if (val <= 0.285156)
        {
            return Colour::fromFloatRGBA(0.21429799999999999, 0.35561900000000002, 0.55118400000000001, 1.0);
        }
        if (val <= 0.289062)
        {
            return Colour::fromFloatRGBA(0.212395, 0.35968299999999997, 0.55171000000000003, 1.0);
        }
        if (val <= 0.292969)
        {
            return Colour::fromFloatRGBA(0.210503, 0.36372700000000002, 0.55220599999999997, 1.0);
        }
        if (val <= 0.296875)
        {
            return Colour::fromFloatRGBA(0.208623, 0.36775200000000002, 0.55267500000000003, 1.0);
        }
        if (val <= 0.300781)
        {
            return Colour::fromFloatRGBA(0.206756, 0.37175799999999998, 0.55311699999999997, 1.0);
        }
        if (val <= 0.304688)
        {
            return Colour::fromFloatRGBA(0.204903, 0.37574600000000002, 0.55353300000000005, 1.0);
        }
        if (val <= 0.308594)
        {
            return Colour::fromFloatRGBA(0.20306299999999999, 0.379716, 0.553925, 1.0);
        }
        if (val <= 0.312500)
        {
            return Colour::fromFloatRGBA(0.201239, 0.38367000000000001, 0.55429399999999995, 1.0);
        }
        if (val <= 0.316406)
        {
            return Colour::fromFloatRGBA(0.19943, 0.38760699999999998, 0.55464199999999997, 1.0);
        }
        if (val <= 0.320312)
        {
            return Colour::fromFloatRGBA(0.19763600000000001, 0.39152799999999999, 0.55496900000000005, 1.0);
        }
        if (val <= 0.324219)
        {
            return Colour::fromFloatRGBA(0.19586000000000001, 0.39543299999999998, 0.55527599999999999, 1.0);
        }
        if (val <= 0.328125)
        {
            return Colour::fromFloatRGBA(0.19409999999999999, 0.39932299999999998, 0.55556499999999998, 1.0);
        }
        if (val <= 0.332031)
        {
            return Colour::fromFloatRGBA(0.192357, 0.40319899999999997, 0.555836, 1.0);
        }
        if (val <= 0.335938)
        {
            return Colour::fromFloatRGBA(0.19063099999999999, 0.40706100000000001, 0.55608900000000006, 1.0);
        }
        if (val <= 0.339844)
        {
            return Colour::fromFloatRGBA(0.18892300000000001, 0.41091, 0.55632599999999999, 1.0);
        }
        if (val <= 0.343750)
        {
            return Colour::fromFloatRGBA(0.18723100000000001, 0.414746, 0.55654700000000001, 1.0);
        }
        if (val <= 0.347656)
        {
            return Colour::fromFloatRGBA(0.185556, 0.41857, 0.55675300000000005, 1.0);
        }
        if (val <= 0.351562)
        {
            return Colour::fromFloatRGBA(0.18389800000000001, 0.42238300000000001, 0.55694399999999999, 1.0);
        }
        if (val <= 0.355469)
        {
            return Colour::fromFloatRGBA(0.182256, 0.42618400000000001, 0.55711999999999995, 1.0);
        }
        if (val <= 0.359375)
        {
            return Colour::fromFloatRGBA(0.18062900000000001, 0.429975, 0.55728200000000006, 1.0);
        }
        if (val <= 0.363281)
        {
            return Colour::fromFloatRGBA(0.17901900000000001, 0.43375599999999997, 0.55742999999999998, 1.0);
        }
        if (val <= 0.367188)
        {
            return Colour::fromFloatRGBA(0.177423, 0.437527, 0.55756499999999998, 1.0);
        }
        if (val <= 0.371094)
        {
            return Colour::fromFloatRGBA(0.175841, 0.44129000000000002, 0.55768499999999999, 1.0);
        }
        if (val <= 0.375000)
        {
            return Colour::fromFloatRGBA(0.17427400000000001, 0.445044, 0.55779199999999995, 1.0);
        }
        if (val <= 0.378906)
        {
            return Colour::fromFloatRGBA(0.17271900000000001, 0.448791, 0.55788499999999996, 1.0);
        }
        if (val <= 0.382812)
        {
            return Colour::fromFloatRGBA(0.17117599999999999, 0.45252999999999999, 0.55796500000000004, 1.0);
        }
        if (val <= 0.386719)
        {
            return Colour::fromFloatRGBA(0.16964599999999999, 0.456262, 0.55803000000000003, 1.0);
        }
        if (val <= 0.390625)
        {
            return Colour::fromFloatRGBA(0.168126, 0.45998800000000001, 0.55808199999999997, 1.0);
        }
        if (val <= 0.394531)
        {
            return Colour::fromFloatRGBA(0.16661699999999999, 0.46370800000000001, 0.55811900000000003, 1.0);
        }
        if (val <= 0.398438)
        {
            return Colour::fromFloatRGBA(0.16511700000000001, 0.46742299999999998, 0.558141, 1.0);
        }
        if (val <= 0.402344)
        {
            return Colour::fromFloatRGBA(0.16362499999999999, 0.47113300000000002, 0.55814799999999998, 1.0);
        }
        if (val <= 0.406250)
        {
            return Colour::fromFloatRGBA(0.16214200000000001, 0.47483799999999998, 0.55813999999999997, 1.0);
        }
        if (val <= 0.410156)
        {
            return Colour::fromFloatRGBA(0.160665, 0.47854000000000002, 0.55811500000000003, 1.0);
        }
        if (val <= 0.414062)
        {
            return Colour::fromFloatRGBA(0.159194, 0.48223700000000003, 0.55807300000000004, 1.0);
        }
        if (val <= 0.417969)
        {
            return Colour::fromFloatRGBA(0.15772900000000001, 0.48593199999999998, 0.55801299999999998, 1.0);
        }
        if (val <= 0.421875)
        {
            return Colour::fromFloatRGBA(0.15626999999999999, 0.489624, 0.55793599999999999, 1.0);
        }
        if (val <= 0.425781)
        {
            return Colour::fromFloatRGBA(0.15481500000000001, 0.493313, 0.55784, 1.0);
        }
        if (val <= 0.429688)
        {
            return Colour::fromFloatRGBA(0.153364, 0.497, 0.557724, 1.0);
        }
        if (val <= 0.433594)
        {
            return Colour::fromFloatRGBA(0.151918, 0.50068500000000005, 0.55758700000000005, 1.0);
        }
        if (val <= 0.437500)
        {
            return Colour::fromFloatRGBA(0.150476, 0.50436899999999996, 0.55742999999999998, 1.0);
        }
        if (val <= 0.441406)
        {
            return Colour::fromFloatRGBA(0.149039, 0.50805100000000003, 0.55725000000000002, 1.0);
        }
        if (val <= 0.445312)
        {
            return Colour::fromFloatRGBA(0.14760699999999999, 0.51173299999999999, 0.55704900000000002, 1.0);
        }
        if (val <= 0.449219)
        {
            return Colour::fromFloatRGBA(0.14618, 0.51541300000000001, 0.55682299999999996, 1.0);
        }
        if (val <= 0.453125)
        {
            return Colour::fromFloatRGBA(0.144759, 0.51909300000000003, 0.55657199999999996, 1.0);
        }
        if (val <= 0.457031)
        {
            return Colour::fromFloatRGBA(0.143343, 0.52277300000000004, 0.55629499999999998, 1.0);
        }
        if (val <= 0.460938)
        {
            return Colour::fromFloatRGBA(0.14193500000000001, 0.52645299999999995, 0.55599100000000001, 1.0);
        }
        if (val <= 0.464844)
        {
            return Colour::fromFloatRGBA(0.14053599999999999, 0.53013200000000005, 0.55565900000000001, 1.0);
        }
        if (val <= 0.468750)
        {
            return Colour::fromFloatRGBA(0.13914699999999999, 0.53381199999999995, 0.55529799999999996, 1.0);
        }
        if (val <= 0.472656)
        {
            return Colour::fromFloatRGBA(0.13777, 0.53749199999999997, 0.55490600000000001, 1.0);
        }
        if (val <= 0.476562)
        {
            return Colour::fromFloatRGBA(0.136408, 0.54117300000000002, 0.55448299999999995, 1.0);
        }
        if (val <= 0.480469)
        {
            return Colour::fromFloatRGBA(0.13506599999999999, 0.54485300000000003, 0.55402899999999999, 1.0);
        }
        if (val <= 0.484375)
        {
            return Colour::fromFloatRGBA(0.133743, 0.54853499999999999, 0.55354099999999995, 1.0);
        }
        if (val <= 0.488281)
        {
            return Colour::fromFloatRGBA(0.13244400000000001, 0.55221600000000004, 0.55301800000000001, 1.0);
        }
        if (val <= 0.492188)
        {
            return Colour::fromFloatRGBA(0.13117200000000001, 0.55589900000000003, 0.55245900000000003, 1.0);
        }
        if (val <= 0.496094)
        {
            return Colour::fromFloatRGBA(0.12993299999999999, 0.55958200000000002, 0.55186400000000002, 1.0);
        }
        if (val <= 0.500000)
        {
            return Colour::fromFloatRGBA(0.12872900000000001, 0.56326500000000002, 0.55122899999999997, 1.0);
        }
        if (val <= 0.503906)
        {
            return Colour::fromFloatRGBA(0.12756799999999999, 0.56694900000000004, 0.55055600000000005, 1.0);
        }
        if (val <= 0.507812)
        {
            return Colour::fromFloatRGBA(0.12645300000000001, 0.57063299999999995, 0.54984100000000002, 1.0);
        }
        if (val <= 0.511719)
        {
            return Colour::fromFloatRGBA(0.12539400000000001, 0.574318, 0.54908599999999996, 1.0);
        }
        if (val <= 0.515625)
        {
            return Colour::fromFloatRGBA(0.12439500000000001, 0.57800200000000002, 0.54828699999999997, 1.0);
        }
        if (val <= 0.519531)
        {
            return Colour::fromFloatRGBA(0.123463, 0.58168699999999995, 0.54744499999999996, 1.0);
        }
        if (val <= 0.523438)
        {
            return Colour::fromFloatRGBA(0.12260600000000001, 0.58537099999999997, 0.54655699999999996, 1.0);
        }
        if (val <= 0.527344)
        {
            return Colour::fromFloatRGBA(0.12183099999999999, 0.589055, 0.54562299999999997, 1.0);
        }
        if (val <= 0.531250)
        {
            return Colour::fromFloatRGBA(0.12114800000000001, 0.59273900000000002, 0.54464100000000004, 1.0);
        }
        if (val <= 0.535156)
        {
            return Colour::fromFloatRGBA(0.12056500000000001, 0.59642200000000001, 0.54361099999999996, 1.0);
        }
        if (val <= 0.539062)
        {
            return Colour::fromFloatRGBA(0.120092, 0.60010399999999997, 0.54252999999999996, 1.0);
        }
        if (val <= 0.542969)
        {
            return Colour::fromFloatRGBA(0.119738, 0.60378500000000002, 0.54139999999999999, 1.0);
        }
        if (val <= 0.546875)
        {
            return Colour::fromFloatRGBA(0.11951199999999999, 0.607464, 0.54021799999999998, 1.0);
        }
        if (val <= 0.550781)
        {
            return Colour::fromFloatRGBA(0.119423, 0.61114100000000005, 0.53898199999999996, 1.0);
        }
        if (val <= 0.554688)
        {
            return Colour::fromFloatRGBA(0.11948300000000001, 0.61481699999999995, 0.53769199999999995, 1.0);
        }
        if (val <= 0.558594)
        {
            return Colour::fromFloatRGBA(0.119699, 0.61848999999999998, 0.53634700000000002, 1.0);
        }
        if (val <= 0.562500)
        {
            return Colour::fromFloatRGBA(0.12008099999999999, 0.62216099999999996, 0.53494600000000003, 1.0);
        }
        if (val <= 0.566406)
        {
            return Colour::fromFloatRGBA(0.120638, 0.62582800000000005, 0.53348799999999996, 1.0);
        }
        if (val <= 0.570312)
        {
            return Colour::fromFloatRGBA(0.12138, 0.62949200000000005, 0.53197300000000003, 1.0);
        }
        if (val <= 0.574219)
        {
            return Colour::fromFloatRGBA(0.122312, 0.63315299999999997, 0.53039800000000004, 1.0);
        }
        if (val <= 0.578125)
        {
            return Colour::fromFloatRGBA(0.123444, 0.63680899999999996, 0.52876299999999998, 1.0);
        }
        if (val <= 0.582031)
        {
            return Colour::fromFloatRGBA(0.12478, 0.64046099999999995, 0.52706799999999998, 1.0);
        }
        if (val <= 0.585938)
        {
            return Colour::fromFloatRGBA(0.12632599999999999, 0.64410699999999999, 0.52531099999999997, 1.0);
        }
        if (val <= 0.589844)
        {
            return Colour::fromFloatRGBA(0.12808700000000001, 0.64774900000000002, 0.52349100000000004, 1.0);
        }
        if (val <= 0.593750)
        {
            return Colour::fromFloatRGBA(0.13006699999999999, 0.65138399999999996, 0.52160799999999996, 1.0);
        }
        if (val <= 0.597656)
        {
            return Colour::fromFloatRGBA(0.132268, 0.65501399999999999, 0.51966100000000004, 1.0);
        }
        if (val <= 0.601562)
        {
            return Colour::fromFloatRGBA(0.13469200000000001, 0.658636, 0.51764900000000003, 1.0);
        }
        if (val <= 0.605469)
        {
            return Colour::fromFloatRGBA(0.13733899999999999, 0.66225199999999995, 0.515571, 1.0);
        }
        if (val <= 0.609375)
        {
            return Colour::fromFloatRGBA(0.14021, 0.66585899999999998, 0.51342699999999997, 1.0);
        }
        if (val <= 0.613281)
        {
            return Colour::fromFloatRGBA(0.14330300000000001, 0.66945900000000003, 0.51121499999999997, 1.0);
        }
        if (val <= 0.617188)
        {
            return Colour::fromFloatRGBA(0.146616, 0.67305000000000004, 0.50893600000000006, 1.0);
        }
        if (val <= 0.621094)
        {
            return Colour::fromFloatRGBA(0.150148, 0.67663099999999998, 0.50658899999999996, 1.0);
        }
        if (val <= 0.625000)
        {
            return Colour::fromFloatRGBA(0.153894, 0.680203, 0.50417199999999995, 1.0);
        }
        if (val <= 0.628906)
        {
            return Colour::fromFloatRGBA(0.15785099999999999, 0.68376499999999996, 0.50168599999999997, 1.0);
        }
        if (val <= 0.632812)
        {
            return Colour::fromFloatRGBA(0.16201599999999999, 0.68731600000000004, 0.49912899999999999, 1.0);
        }
        if (val <= 0.636719)
        {
            return Colour::fromFloatRGBA(0.166383, 0.69085600000000003, 0.496502, 1.0);
        }
        if (val <= 0.640625)
        {
            return Colour::fromFloatRGBA(0.17094799999999999, 0.694384, 0.49380299999999999, 1.0);
        }
        if (val <= 0.644531)
        {
            return Colour::fromFloatRGBA(0.175707, 0.69789999999999996, 0.491033, 1.0);
        }
        if (val <= 0.648438)
        {
            return Colour::fromFloatRGBA(0.18065300000000001, 0.70140199999999997, 0.48818899999999998, 1.0);
        }
        if (val <= 0.652344)
        {
            return Colour::fromFloatRGBA(0.185783, 0.70489100000000005, 0.48527300000000001, 1.0);
        }
        if (val <= 0.656250)
        {
            return Colour::fromFloatRGBA(0.19109000000000001, 0.70836600000000005, 0.48228399999999999, 1.0);
        }
        if (val <= 0.660156)
        {
            return Colour::fromFloatRGBA(0.196571, 0.71182699999999999, 0.47922100000000001, 1.0);
        }
        if (val <= 0.664062)
        {
            return Colour::fromFloatRGBA(0.20221900000000001, 0.71527200000000002, 0.47608400000000001, 1.0);
        }
        if (val <= 0.667969)
        {
            return Colour::fromFloatRGBA(0.20802999999999999, 0.71870100000000003, 0.47287299999999999, 1.0);
        }
        if (val <= 0.671875)
        {
            return Colour::fromFloatRGBA(0.214, 0.72211400000000003, 0.46958800000000001, 1.0);
        }
        if (val <= 0.675781)
        {
            return Colour::fromFloatRGBA(0.22012399999999999, 0.72550899999999996, 0.46622599999999997, 1.0);
        }
        if (val <= 0.679688)
        {
            return Colour::fromFloatRGBA(0.22639699999999999, 0.72888799999999998, 0.46278900000000001, 1.0);
        }
        if (val <= 0.683594)
        {
            return Colour::fromFloatRGBA(0.23281499999999999, 0.73224699999999998, 0.45927699999999999, 1.0);
        }
        if (val <= 0.687500)
        {
            return Colour::fromFloatRGBA(0.239374, 0.73558800000000002, 0.45568799999999998, 1.0);
        }
        if (val <= 0.691406)
        {
            return Colour::fromFloatRGBA(0.24607000000000001, 0.73890999999999996, 0.45202399999999998, 1.0);
        }
        if (val <= 0.695312)
        {
            return Colour::fromFloatRGBA(0.25289899999999998, 0.74221099999999995, 0.44828400000000002, 1.0);
        }
        if (val <= 0.699219)
        {
            return Colour::fromFloatRGBA(0.259857, 0.74549200000000004, 0.444467, 1.0);
        }
        if (val <= 0.703125)
        {
            return Colour::fromFloatRGBA(0.26694099999999998, 0.74875100000000006, 0.44057299999999999, 1.0);
        }
        if (val <= 0.707031)
        {
            return Colour::fromFloatRGBA(0.27414899999999998, 0.75198799999999999, 0.43660100000000002, 1.0);
        }
        if (val <= 0.710938)
        {
            return Colour::fromFloatRGBA(0.28147699999999998, 0.75520299999999996, 0.43255199999999999, 1.0);
        }
        if (val <= 0.714844)
        {
            return Colour::fromFloatRGBA(0.28892099999999998, 0.75839400000000001, 0.42842599999999997, 1.0);
        }
        if (val <= 0.718750)
        {
            return Colour::fromFloatRGBA(0.29647899999999999, 0.76156100000000004, 0.42422300000000002, 1.0);
        }
        if (val <= 0.722656)
        {
            return Colour::fromFloatRGBA(0.30414799999999997, 0.76470400000000005, 0.41994300000000001, 1.0);
        }
        if (val <= 0.726562)
        {
            return Colour::fromFloatRGBA(0.31192500000000001, 0.767822, 0.41558600000000001, 1.0);
        }
        if (val <= 0.730469)
        {
            return Colour::fromFloatRGBA(0.31980900000000001, 0.77091399999999999, 0.41115200000000002, 1.0);
        }
        if (val <= 0.734375)
        {
            return Colour::fromFloatRGBA(0.32779599999999998, 0.77398, 0.40664, 1.0);
        }
        if (val <= 0.738281)
        {
            return Colour::fromFloatRGBA(0.33588499999999999, 0.77701799999999999, 0.40204899999999999, 1.0);
        }
        if (val <= 0.742188)
        {
            return Colour::fromFloatRGBA(0.34407399999999999, 0.78002899999999997, 0.39738099999999998, 1.0);
        }
        if (val <= 0.746094)
        {
            return Colour::fromFloatRGBA(0.35236000000000001, 0.78301100000000001, 0.39263599999999999, 1.0);
        }
        if (val <= 0.750000)
        {
            return Colour::fromFloatRGBA(0.36074099999999998, 0.785964, 0.38781399999999999, 1.0);
        }
        if (val <= 0.753906)
        {
            return Colour::fromFloatRGBA(0.36921399999999999, 0.78888800000000003, 0.38291399999999998, 1.0);
        }
        if (val <= 0.757812)
        {
            return Colour::fromFloatRGBA(0.37777899999999998, 0.79178099999999996, 0.37793900000000002, 1.0);
        }
        if (val <= 0.761719)
        {
            return Colour::fromFloatRGBA(0.38643300000000003, 0.79464400000000002, 0.372886, 1.0);
        }
        if (val <= 0.765625)
        {
            return Colour::fromFloatRGBA(0.39517400000000003, 0.79747500000000004, 0.367757, 1.0);
        }
        if (val <= 0.769531)
        {
            return Colour::fromFloatRGBA(0.404001, 0.80027499999999996, 0.36255199999999999, 1.0);
        }
        if (val <= 0.773438)
        {
            return Colour::fromFloatRGBA(0.41291299999999997, 0.803041, 0.357269, 1.0);
        }
        if (val <= 0.777344)
        {
            return Colour::fromFloatRGBA(0.42190800000000001, 0.80577399999999999, 0.35191, 1.0);
        }
        if (val <= 0.781250)
        {
            return Colour::fromFloatRGBA(0.430983, 0.808473, 0.34647600000000001, 1.0);
        }
        if (val <= 0.785156)
        {
            return Colour::fromFloatRGBA(0.440137, 0.81113800000000003, 0.34096700000000002, 1.0);
        }
        if (val <= 0.789062)
        {
            return Colour::fromFloatRGBA(0.44936799999999999, 0.81376800000000005, 0.33538400000000002, 1.0);
        }
        if (val <= 0.792969)
        {
            return Colour::fromFloatRGBA(0.45867400000000003, 0.81636299999999995, 0.32972699999999999, 1.0);
        }
        if (val <= 0.796875)
        {
            return Colour::fromFloatRGBA(0.468053, 0.81892100000000001, 0.32399800000000001, 1.0);
        }
        if (val <= 0.800781)
        {
            return Colour::fromFloatRGBA(0.47750399999999998, 0.82144399999999995, 0.31819500000000001, 1.0);
        }
        if (val <= 0.804688)
        {
            return Colour::fromFloatRGBA(0.48702600000000001, 0.82392900000000002, 0.31232100000000002, 1.0);
        }
        if (val <= 0.808594)
        {
            return Colour::fromFloatRGBA(0.49661499999999997, 0.826376, 0.30637700000000001, 1.0);
        }
        if (val <= 0.812500)
        {
            return Colour::fromFloatRGBA(0.50627100000000003, 0.82878600000000002, 0.30036200000000002, 1.0);
        }
        if (val <= 0.816406)
        {
            return Colour::fromFloatRGBA(0.51599200000000001, 0.83115799999999995, 0.29427900000000001, 1.0);
        }
        if (val <= 0.820312)
        {
            return Colour::fromFloatRGBA(0.52577600000000002, 0.83349099999999998, 0.28812700000000002, 1.0);
        }
        if (val <= 0.824219)
        {
            return Colour::fromFloatRGBA(0.53562100000000001, 0.835785, 0.28190799999999999, 1.0);
        }
        if (val <= 0.828125)
        {
            return Colour::fromFloatRGBA(0.54552400000000001, 0.83803899999999998, 0.27562599999999998, 1.0);
        }
        if (val <= 0.832031)
        {
            return Colour::fromFloatRGBA(0.55548399999999998, 0.84025399999999995, 0.26928099999999999, 1.0);
        }
        if (val <= 0.835938)
        {
            return Colour::fromFloatRGBA(0.56549799999999995, 0.84243000000000001, 0.26287700000000003, 1.0);
        }
        if (val <= 0.839844)
        {
            return Colour::fromFloatRGBA(0.57556300000000005, 0.84456600000000004, 0.256415, 1.0);
        }
        if (val <= 0.843750)
        {
            return Colour::fromFloatRGBA(0.58567800000000003, 0.846661, 0.24989700000000001, 1.0);
        }
        if (val <= 0.847656)
        {
            return Colour::fromFloatRGBA(0.59583900000000001, 0.84871700000000005, 0.24332899999999999, 1.0);
        }
        if (val <= 0.851562)
        {
            return Colour::fromFloatRGBA(0.60604499999999994, 0.85073299999999996, 0.23671200000000001, 1.0);
        }
        if (val <= 0.855469)
        {
            return Colour::fromFloatRGBA(0.61629299999999998, 0.85270900000000005, 0.23005200000000001, 1.0);
        }
        if (val <= 0.859375)
        {
            return Colour::fromFloatRGBA(0.626579, 0.85464499999999999, 0.223353, 1.0);
        }
        if (val <= 0.863281)
        {
            return Colour::fromFloatRGBA(0.63690199999999997, 0.85654200000000003, 0.21662000000000001, 1.0);
        }
        if (val <= 0.867188)
        {
            return Colour::fromFloatRGBA(0.64725699999999997, 0.85840000000000005, 0.20986099999999999, 1.0);
        }
        if (val <= 0.871094)
        {
            return Colour::fromFloatRGBA(0.65764199999999995, 0.86021899999999996, 0.20308200000000001, 1.0);
        }
        if (val <= 0.875000)
        {
            return Colour::fromFloatRGBA(0.66805400000000004, 0.86199899999999996, 0.196293, 1.0);
        }
        if (val <= 0.878906)
        {
            return Colour::fromFloatRGBA(0.67848900000000001, 0.86374200000000001, 0.189503, 1.0);
        }
        if (val <= 0.882812)
        {
            return Colour::fromFloatRGBA(0.688944, 0.865448, 0.182725, 1.0);
        }
        if (val <= 0.886719)
        {
            return Colour::fromFloatRGBA(0.69941500000000001, 0.86711700000000003, 0.17597099999999999, 1.0);
        }
        if (val <= 0.890625)
        {
            return Colour::fromFloatRGBA(0.70989800000000003, 0.86875100000000005, 0.16925699999999999, 1.0);
        }
        if (val <= 0.894531)
        {
            return Colour::fromFloatRGBA(0.720391, 0.87034999999999996, 0.162603, 1.0);
        }
        if (val <= 0.898438)
        {
            return Colour::fromFloatRGBA(0.73088900000000001, 0.87191600000000002, 0.156029, 1.0);
        }
        if (val <= 0.902344)
        {
            return Colour::fromFloatRGBA(0.74138800000000005, 0.87344900000000003, 0.149561, 1.0);
        }
        if (val <= 0.906250)
        {
            return Colour::fromFloatRGBA(0.751884, 0.87495100000000003, 0.14322799999999999, 1.0);
        }
        if (val <= 0.910156)
        {
            return Colour::fromFloatRGBA(0.76237299999999997, 0.87642399999999998, 0.13706399999999999, 1.0);
        }
        if (val <= 0.914062)
        {
            return Colour::fromFloatRGBA(0.77285199999999998, 0.87786799999999998, 0.131109, 1.0);
        }
        if (val <= 0.917969)
        {
            return Colour::fromFloatRGBA(0.78331499999999998, 0.87928499999999998, 0.12540499999999999, 1.0);
        }
        if (val <= 0.921875)
        {
            return Colour::fromFloatRGBA(0.79376000000000002, 0.88067799999999996, 0.120005, 1.0);
        }
        if (val <= 0.925781)
        {
            return Colour::fromFloatRGBA(0.80418199999999995, 0.882046, 0.114965, 1.0);
        }
        if (val <= 0.929688)
        {
            return Colour::fromFloatRGBA(0.81457599999999997, 0.88339299999999998, 0.110347, 1.0);
        }
        if (val <= 0.933594)
        {
            return Colour::fromFloatRGBA(0.82494000000000001, 0.88471999999999995, 0.10621700000000001, 1.0);
        }
        if (val <= 0.937500)
        {
            return Colour::fromFloatRGBA(0.83526999999999996, 0.88602899999999996, 0.102646, 1.0);
        }
        if (val <= 0.941406)
        {
            return Colour::fromFloatRGBA(0.84556100000000001, 0.88732200000000006, 0.099701999999999999, 1.0);
        }
        if (val <= 0.945312)
        {
            return Colour::fromFloatRGBA(0.85580999999999996, 0.88860099999999997, 0.097451999999999997, 1.0);
        }
        if (val <= 0.949219)
        {
            return Colour::fromFloatRGBA(0.86601300000000003, 0.88986799999999999, 0.095952999999999997, 1.0);
        }
        if (val <= 0.953125)
        {
            return Colour::fromFloatRGBA(0.87616799999999995, 0.89112499999999994, 0.095250000000000001, 1.0);
        }
        if (val <= 0.957031)
        {
            return Colour::fromFloatRGBA(0.88627100000000003, 0.892374, 0.095374, 1.0);
        }
        if (val <= 0.960938)
        {
            return Colour::fromFloatRGBA(0.89632000000000001, 0.89361599999999997, 0.096335000000000004, 1.0);
        }
        if (val <= 0.964844)
        {
            return Colour::fromFloatRGBA(0.90631099999999998, 0.89485499999999996, 0.098125000000000004, 1.0);
        }
        if (val <= 0.968750)
        {
            return Colour::fromFloatRGBA(0.916242, 0.89609099999999997, 0.100717, 1.0);
        }
        if (val <= 0.972656)
        {
            return Colour::fromFloatRGBA(0.92610599999999998, 0.89732999999999996, 0.104071, 1.0);
        }
        if (val <= 0.976562)
        {
            return Colour::fromFloatRGBA(0.93590399999999996, 0.89856999999999998, 0.108131, 1.0);
        }
        if (val <= 0.980469)
        {
            return Colour::fromFloatRGBA(0.94563600000000003, 0.89981500000000003, 0.11283799999999999, 1.0);
        }
        if (val <= 0.984375)
        {
            return Colour::fromFloatRGBA(0.95530000000000004, 0.901065, 0.118128, 1.0);
        }
        if (val <= 0.988281)
        {
            return Colour::fromFloatRGBA(0.96489400000000003, 0.90232299999999999, 0.123941, 1.0);
        }
        if (val <= 0.992188)
        {
            return Colour::fromFloatRGBA(0.97441699999999998, 0.90359, 0.130215, 1.0);
        }
        if (val <= 0.996094)
        {
            return Colour::fromFloatRGBA(0.98386799999999996, 0.90486699999999998, 0.13689699999999999, 1.0);
        }

        return Colour::fromFloatRGBA(0.99324800000000002, 0.90615699999999999, 0.14393600000000001, 1.0);
    }

    Colour colourFromJet(float val)
    {
        if (val <= 0.003906)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.5, 1.0);
        }
        if (val <= 0.007812)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.517825311942959, 1.0);
        }
        if (val <= 0.011719)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.535650623885918, 1.0);
        }
        if (val <= 0.015625)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.553475935828877, 1.0);
        }
        if (val <= 0.019531)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.571301247771836, 1.0);
        }
        if (val <= 0.023438)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.589126559714795, 1.0);
        }
        if (val <= 0.027344)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.60695187165775399, 1.0);
        }
        if (val <= 0.031250)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.62477718360071299, 1.0);
        }
        if (val <= 0.035156)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.64260249554367199, 1.0);
        }
        if (val <= 0.039062)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.66042780748663099, 1.0);
        }
        if (val <= 0.042969)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.67825311942958999, 1.0);
        }
        if (val <= 0.046875)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.69607843137254899, 1.0);
        }
        if (val <= 0.050781)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.71390374331550799, 1.0);
        }
        if (val <= 0.054688)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.73172905525846699, 1.0);
        }
        if (val <= 0.058594)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.74955436720142599, 1.0);
        }
        if (val <= 0.062500)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.76737967914438499, 1.0);
        }
        if (val <= 0.066406)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.78520499108734398, 1.0);
        }
        if (val <= 0.070312)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.80303030303030298, 1.0);
        }
        if (val <= 0.074219)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.82085561497326198, 1.0);
        }
        if (val <= 0.078125)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.83868092691622098, 1.0);
        }
        if (val <= 0.082031)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.85650623885917998, 1.0);
        }
        if (val <= 0.085938)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.87433155080213898, 1.0);
        }
        if (val <= 0.089844)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.89215686274509798, 1.0);
        }
        if (val <= 0.093750)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.90998217468805698, 1.0);
        }
        if (val <= 0.097656)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.92780748663101598, 1.0);
        }
        if (val <= 0.101562)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.94563279857397498, 1.0);
        }
        if (val <= 0.105469)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.96345811051693397, 1.0);
        }
        if (val <= 0.109375)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.98128342245989297, 1.0);
        }
        if (val <= 0.113281)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 0.99910873440285197, 1.0);
        }
        if (val <= 0.117188)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 1.0, 1.0);
        }
        if (val <= 0.121094)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 1.0, 1.0);
        }
        if (val <= 0.125000)
        {
            return Colour::fromFloatRGBA(0.0, 0.0, 1.0, 1.0);
        }
        if (val <= 0.128906)
        {
            return Colour::fromFloatRGBA(0.0, 0.0019607843137254902, 1.0, 1.0);
        }
        if (val <= 0.132812)
        {
            return Colour::fromFloatRGBA(0.0, 0.0176470588235293, 1.0, 1.0);
        }
        if (val <= 0.136719)
        {
            return Colour::fromFloatRGBA(0.0, 0.033333333333333333, 1.0, 1.0);
        }
        if (val <= 0.140625)
        {
            return Colour::fromFloatRGBA(0.0, 0.049019607843137254, 1.0, 1.0);
        }
        if (val <= 0.144531)
        {
            return Colour::fromFloatRGBA(0.0, 0.064705882352941183, 1.0, 1.0);
        }
        if (val <= 0.148438)
        {
            return Colour::fromFloatRGBA(0.0, 0.080392156862744993, 1.0, 1.0);
        }
        if (val <= 0.152344)
        {
            return Colour::fromFloatRGBA(0.0, 0.096078431372549025, 1.0, 1.0);
        }
        if (val <= 0.156250)
        {
            return Colour::fromFloatRGBA(0.0, 0.11176470588235295, 1.0, 1.0);
        }
        if (val <= 0.160156)
        {
            return Colour::fromFloatRGBA(0.0, 0.12745098039215685, 1.0, 1.0);
        }
        if (val <= 0.164062)
        {
            return Colour::fromFloatRGBA(0.0, 0.14313725490196066, 1.0, 1.0);
        }
        if (val <= 0.167969)
        {
            return Colour::fromFloatRGBA(0.0, 0.1588235294117647, 1.0, 1.0);
        }
        if (val <= 0.171875)
        {
            return Colour::fromFloatRGBA(0.0, 0.17450980392156862, 1.0, 1.0);
        }
        if (val <= 0.175781)
        {
            return Colour::fromFloatRGBA(0.0, 0.19019607843137254, 1.0, 1.0);
        }
        if (val <= 0.179688)
        {
            return Colour::fromFloatRGBA(0.0, 0.20588235294117635, 1.0, 1.0);
        }
        if (val <= 0.183594)
        {
            return Colour::fromFloatRGBA(0.0, 0.22156862745098038, 1.0, 1.0);
        }
        if (val <= 0.187500)
        {
            return Colour::fromFloatRGBA(0.0, 0.2372549019607843, 1.0, 1.0);
        }
        if (val <= 0.191406)
        {
            return Colour::fromFloatRGBA(0.0, 0.25294117647058822, 1.0, 1.0);
        }
        if (val <= 0.195312)
        {
            return Colour::fromFloatRGBA(0.0, 0.26862745098039204, 1.0, 1.0);
        }
        if (val <= 0.199219)
        {
            return Colour::fromFloatRGBA(0.0, 0.28431372549019607, 1.0, 1.0);
        }
        if (val <= 0.203125)
        {
            return Colour::fromFloatRGBA(0.0, 0.29999999999999999, 1.0, 1.0);
        }
        if (val <= 0.207031)
        {
            return Colour::fromFloatRGBA(0.0, 0.31568627450980391, 1.0, 1.0);
        }
        if (val <= 0.210938)
        {
            return Colour::fromFloatRGBA(0.0, 0.33137254901960772, 1.0, 1.0);
        }
        if (val <= 0.214844)
        {
            return Colour::fromFloatRGBA(0.0, 0.34705882352941175, 1.0, 1.0);
        }
        if (val <= 0.218750)
        {
            return Colour::fromFloatRGBA(0.0, 0.36274509803921567, 1.0, 1.0);
        }
        if (val <= 0.222656)
        {
            return Colour::fromFloatRGBA(0.0, 0.3784313725490196, 1.0, 1.0);
        }
        if (val <= 0.226562)
        {
            return Colour::fromFloatRGBA(0.0, 0.39411764705882341, 1.0, 1.0);
        }
        if (val <= 0.230469)
        {
            return Colour::fromFloatRGBA(0.0, 0.40980392156862744, 1.0, 1.0);
        }
        if (val <= 0.234375)
        {
            return Colour::fromFloatRGBA(0.0, 0.42549019607843136, 1.0, 1.0);
        }
        if (val <= 0.238281)
        {
            return Colour::fromFloatRGBA(0.0, 0.44117647058823528, 1.0, 1.0);
        }
        if (val <= 0.242188)
        {
            return Colour::fromFloatRGBA(0.0, 0.45686274509803909, 1.0, 1.0);
        }
        if (val <= 0.246094)
        {
            return Colour::fromFloatRGBA(0.0, 0.47254901960784312, 1.0, 1.0);
        }
        if (val <= 0.250000)
        {
            return Colour::fromFloatRGBA(0.0, 0.48823529411764705, 1.0, 1.0);
        }
        if (val <= 0.253906)
        {
            return Colour::fromFloatRGBA(0.0, 0.50392156862745097, 1.0, 1.0);
        }
        if (val <= 0.257812)
        {
            return Colour::fromFloatRGBA(0.0, 0.51960784313725494, 1.0, 1.0);
        }
        if (val <= 0.261719)
        {
            return Colour::fromFloatRGBA(0.0, 0.53529411764705859, 1.0, 1.0);
        }
        if (val <= 0.265625)
        {
            return Colour::fromFloatRGBA(0.0, 0.55098039215686279, 1.0, 1.0);
        }
        if (val <= 0.269531)
        {
            return Colour::fromFloatRGBA(0.0, 0.56666666666666665, 1.0, 1.0);
        }
        if (val <= 0.273438)
        {
            return Colour::fromFloatRGBA(0.0, 0.58235294117647063, 1.0, 1.0);
        }
        if (val <= 0.277344)
        {
            return Colour::fromFloatRGBA(0.0, 0.59803921568627449, 1.0, 1.0);
        }
        if (val <= 0.281250)
        {
            return Colour::fromFloatRGBA(0.0, 0.61372549019607847, 1.0, 1.0);
        }
        if (val <= 0.285156)
        {
            return Colour::fromFloatRGBA(0.0, 0.62941176470588234, 1.0, 1.0);
        }
        if (val <= 0.289062)
        {
            return Colour::fromFloatRGBA(0.0, 0.64509803921568631, 1.0, 1.0);
        }
        if (val <= 0.292969)
        {
            return Colour::fromFloatRGBA(0.0, 0.66078431372548996, 1.0, 1.0);
        }
        if (val <= 0.296875)
        {
            return Colour::fromFloatRGBA(0.0, 0.67647058823529416, 1.0, 1.0);
        }
        if (val <= 0.300781)
        {
            return Colour::fromFloatRGBA(0.0, 0.69215686274509802, 1.0, 1.0);
        }
        if (val <= 0.304688)
        {
            return Colour::fromFloatRGBA(0.0, 0.707843137254902, 1.0, 1.0);
        }
        if (val <= 0.308594)
        {
            return Colour::fromFloatRGBA(0.0, 0.72352941176470587, 1.0, 1.0);
        }
        if (val <= 0.312500)
        {
            return Colour::fromFloatRGBA(0.0, 0.73921568627450984, 1.0, 1.0);
        }
        if (val <= 0.316406)
        {
            return Colour::fromFloatRGBA(0.0, 0.75490196078431371, 1.0, 1.0);
        }
        if (val <= 0.320312)
        {
            return Colour::fromFloatRGBA(0.0, 0.77058823529411768, 1.0, 1.0);
        }
        if (val <= 0.324219)
        {
            return Colour::fromFloatRGBA(0.0, 0.78627450980392133, 1.0, 1.0);
        }
        if (val <= 0.328125)
        {
            return Colour::fromFloatRGBA(0.0, 0.80196078431372553, 1.0, 1.0);
        }
        if (val <= 0.332031)
        {
            return Colour::fromFloatRGBA(0.0, 0.81764705882352939, 1.0, 1.0);
        }
        if (val <= 0.335938)
        {
            return Colour::fromFloatRGBA(0.0, 0.83333333333333337, 1.0, 1.0);
        }
        if (val <= 0.339844)
        {
            return Colour::fromFloatRGBA(0.0, 0.84901960784313724, 1.0, 1.0);
        }
        if (val <= 0.343750)
        {
            return Colour::fromFloatRGBA(0.0, 0.86470588235294121, 0.99620493358633777, 1.0);
        }
        if (val <= 0.347656)
        {
            return Colour::fromFloatRGBA(0.0, 0.88039215686274508, 0.98355471220746371, 1.0);
        }
        if (val <= 0.351562)
        {
            return Colour::fromFloatRGBA(0.0, 0.89607843137254906, 0.97090449082858954, 1.0);
        }
        if (val <= 0.355469)
        {
            return Colour::fromFloatRGBA(0.0094876660341554168, 0.9117647058823527, 0.95825426944971559, 1.0);
        }
        if (val <= 0.359375)
        {
            return Colour::fromFloatRGBA(0.022137887413029723, 0.9274509803921569, 0.94560404807084131, 1.0);
        }
        if (val <= 0.363281)
        {
            return Colour::fromFloatRGBA(0.034788108791903853, 0.94313725490196076, 0.93295382669196714, 1.0);
        }
        if (val <= 0.367188)
        {
            return Colour::fromFloatRGBA(0.047438330170777983, 0.95882352941176474, 0.92030360531309297, 1.0);
        }
        if (val <= 0.371094)
        {
            return Colour::fromFloatRGBA(0.060088551549652112, 0.97450980392156861, 0.9076533839342189, 1.0);
        }
        if (val <= 0.375000)
        {
            return Colour::fromFloatRGBA(0.072738772928526235, 0.99019607843137258, 0.89500316255534473, 1.0);
        }
        if (val <= 0.378906)
        {
            return Colour::fromFloatRGBA(0.085388994307400365, 1.0, 0.88235294117647056, 1.0);
        }
        if (val <= 0.382812)
        {
            return Colour::fromFloatRGBA(0.098039215686274495, 1.0, 0.8697027197975965, 1.0);
        }
        if (val <= 0.386719)
        {
            return Colour::fromFloatRGBA(0.11068943706514844, 1.0, 0.85705249841872255, 1.0);
        }
        if (val <= 0.390625)
        {
            return Colour::fromFloatRGBA(0.12333965844402275, 1.0, 0.84440227703984827, 1.0);
        }
        if (val <= 0.394531)
        {
            return Colour::fromFloatRGBA(0.13598987982289687, 1.0, 0.8317520556609741, 1.0);
        }
        if (val <= 0.398438)
        {
            return Colour::fromFloatRGBA(0.14864010120177101, 1.0, 0.81910183428209993, 1.0);
        }
        if (val <= 0.402344)
        {
            return Colour::fromFloatRGBA(0.16129032258064513, 1.0, 0.80645161290322587, 1.0);
        }
        if (val <= 0.406250)
        {
            return Colour::fromFloatRGBA(0.17394054395951927, 1.0, 0.7938013915243517, 1.0);
        }
        if (val <= 0.410156)
        {
            return Colour::fromFloatRGBA(0.18659076533839339, 1.0, 0.78115117014547764, 1.0);
        }
        if (val <= 0.414062)
        {
            return Colour::fromFloatRGBA(0.19924098671726753, 1.0, 0.76850094876660346, 1.0);
        }
        if (val <= 0.417969)
        {
            return Colour::fromFloatRGBA(0.21189120809614148, 1.0, 0.75585072738772952, 1.0);
        }
        if (val <= 0.421875)
        {
            return Colour::fromFloatRGBA(0.22454142947501579, 1.0, 0.74320050600885512, 1.0);
        }
        if (val <= 0.425781)
        {
            return Colour::fromFloatRGBA(0.23719165085388991, 1.0, 0.73055028462998106, 1.0);
        }
        if (val <= 0.429688)
        {
            return Colour::fromFloatRGBA(0.24984187223276405, 1.0, 0.717900063251107, 1.0);
        }
        if (val <= 0.433594)
        {
            return Colour::fromFloatRGBA(0.26249209361163817, 1.0, 0.70524984187223283, 1.0);
        }
        if (val <= 0.437500)
        {
            return Colour::fromFloatRGBA(0.27514231499051228, 1.0, 0.69259962049335866, 1.0);
        }
        if (val <= 0.441406)
        {
            return Colour::fromFloatRGBA(0.2877925363693864, 1.0, 0.67994939911448449, 1.0);
        }
        if (val <= 0.445312)
        {
            return Colour::fromFloatRGBA(0.30044275774826057, 1.0, 0.66729917773561032, 1.0);
        }
        if (val <= 0.449219)
        {
            return Colour::fromFloatRGBA(0.31309297912713452, 1.0, 0.65464895635673637, 1.0);
        }
        if (val <= 0.453125)
        {
            return Colour::fromFloatRGBA(0.3257432005060088, 1.0, 0.6419987349778622, 1.0);
        }
        if (val <= 0.457031)
        {
            return Colour::fromFloatRGBA(0.33839342188488292, 1.0, 0.62934851359898802, 1.0);
        }
        if (val <= 0.460938)
        {
            return Colour::fromFloatRGBA(0.35104364326375709, 1.0, 0.61669829222011385, 1.0);
        }
        if (val <= 0.464844)
        {
            return Colour::fromFloatRGBA(0.3636938646426312, 1.0, 0.60404807084123968, 1.0);
        }
        if (val <= 0.468750)
        {
            return Colour::fromFloatRGBA(0.37634408602150532, 1.0, 0.59139784946236562, 1.0);
        }
        if (val <= 0.472656)
        {
            return Colour::fromFloatRGBA(0.38899430740037944, 1.0, 0.57874762808349156, 1.0);
        }
        if (val <= 0.476562)
        {
            return Colour::fromFloatRGBA(0.40164452877925361, 1.0, 0.56609740670461739, 1.0);
        }
        if (val <= 0.480469)
        {
            return Colour::fromFloatRGBA(0.4142947501581275, 1.0, 0.55344718532574344, 1.0);
        }
        if (val <= 0.484375)
        {
            return Colour::fromFloatRGBA(0.42694497153700184, 1.0, 0.54079696394686905, 1.0);
        }
        if (val <= 0.488281)
        {
            return Colour::fromFloatRGBA(0.43959519291587595, 1.0, 0.52814674256799488, 1.0);
        }
        if (val <= 0.492188)
        {
            return Colour::fromFloatRGBA(0.45224541429475007, 1.0, 0.51549652118912082, 1.0);
        }
        if (val <= 0.496094)
        {
            return Colour::fromFloatRGBA(0.46489563567362424, 1.0, 0.50284629981024676, 1.0);
        }
        if (val <= 0.500000)
        {
            return Colour::fromFloatRGBA(0.47754585705249836, 1.0, 0.49019607843137258, 1.0);
        }
        if (val <= 0.503906)
        {
            return Colour::fromFloatRGBA(0.49019607843137247, 1.0, 0.47754585705249841, 1.0);
        }
        if (val <= 0.507812)
        {
            return Colour::fromFloatRGBA(0.50284629981024664, 1.0, 0.46489563567362435, 1.0);
        }
        if (val <= 0.511719)
        {
            return Colour::fromFloatRGBA(0.5154965211891207, 1.0, 0.45224541429475018, 1.0);
        }
        if (val <= 0.515625)
        {
            return Colour::fromFloatRGBA(0.52814674256799488, 1.0, 0.43959519291587601, 1.0);
        }
        if (val <= 0.519531)
        {
            return Colour::fromFloatRGBA(0.5407969639468686, 1.0, 0.42694497153700228, 1.0);
        }
        if (val <= 0.523438)
        {
            return Colour::fromFloatRGBA(0.55344718532574311, 1.0, 0.41429475015812778, 1.0);
        }
        if (val <= 0.527344)
        {
            return Colour::fromFloatRGBA(0.56609740670461728, 1.0, 0.40164452877925361, 1.0);
        }
        if (val <= 0.531250)
        {
            return Colour::fromFloatRGBA(0.57874762808349134, 1.0, 0.38899430740037955, 1.0);
        }
        if (val <= 0.535156)
        {
            return Colour::fromFloatRGBA(0.59139784946236551, 1.0, 0.37634408602150538, 1.0);
        }
        if (val <= 0.539062)
        {
            return Colour::fromFloatRGBA(0.60404807084123968, 1.0, 0.3636938646426312, 1.0);
        }
        if (val <= 0.542969)
        {
            return Colour::fromFloatRGBA(0.61669829222011374, 1.0, 0.35104364326375714, 1.0);
        }
        if (val <= 0.546875)
        {
            return Colour::fromFloatRGBA(0.62934851359898791, 1.0, 0.33839342188488297, 1.0);
        }
        if (val <= 0.550781)
        {
            return Colour::fromFloatRGBA(0.64199873497786197, 1.0, 0.32574320050600891, 1.0);
        }
        if (val <= 0.554688)
        {
            return Colour::fromFloatRGBA(0.65464895635673614, 1.0, 0.31309297912713474, 1.0);
        }
        if (val <= 0.558594)
        {
            return Colour::fromFloatRGBA(0.66729917773561032, 1.0, 0.30044275774826057, 1.0);
        }
        if (val <= 0.562500)
        {
            return Colour::fromFloatRGBA(0.67994939911448438, 1.0, 0.28779253636938651, 1.0);
        }
        if (val <= 0.566406)
        {
            return Colour::fromFloatRGBA(0.69259962049335855, 1.0, 0.27514231499051234, 1.0);
        }
        if (val <= 0.570312)
        {
            return Colour::fromFloatRGBA(0.70524984187223261, 1.0, 0.26249209361163817, 1.0);
        }
        if (val <= 0.574219)
        {
            return Colour::fromFloatRGBA(0.71790006325110678, 1.0, 0.24984187223276411, 1.0);
        }
        if (val <= 0.578125)
        {
            return Colour::fromFloatRGBA(0.73055028462998095, 1.0, 0.23719165085388993, 1.0);
        }
        if (val <= 0.582031)
        {
            return Colour::fromFloatRGBA(0.74320050600885468, 1.0, 0.22454142947501621, 1.0);
        }
        if (val <= 0.585938)
        {
            return Colour::fromFloatRGBA(0.75585072738772918, 1.0, 0.2118912080961417, 1.0);
        }
        if (val <= 0.589844)
        {
            return Colour::fromFloatRGBA(0.76850094876660335, 1.0, 0.19924098671726753, 1.0);
        }
        if (val <= 0.593750)
        {
            return Colour::fromFloatRGBA(0.78115117014547741, 1.0, 0.18659076533839347, 1.0);
        }
        if (val <= 0.597656)
        {
            return Colour::fromFloatRGBA(0.79380139152435159, 1.0, 0.1739405439595193, 1.0);
        }
        if (val <= 0.601562)
        {
            return Colour::fromFloatRGBA(0.80645161290322565, 1.0, 0.16129032258064513, 1.0);
        }
        if (val <= 0.605469)
        {
            return Colour::fromFloatRGBA(0.81910183428209982, 1.0, 0.14864010120177107, 1.0);
        }
        if (val <= 0.609375)
        {
            return Colour::fromFloatRGBA(0.83175205566097399, 1.0, 0.1359898798228969, 1.0);
        }
        if (val <= 0.613281)
        {
            return Colour::fromFloatRGBA(0.84440227703984805, 1.0, 0.12333965844402273, 1.0);
        }
        if (val <= 0.617188)
        {
            return Colour::fromFloatRGBA(0.85705249841872222, 1.0, 0.11068943706514867, 1.0);
        }
        if (val <= 0.621094)
        {
            return Colour::fromFloatRGBA(0.86970271979759628, 1.0, 0.098039215686274495, 1.0);
        }
        if (val <= 0.625000)
        {
            return Colour::fromFloatRGBA(0.88235294117647045, 1.0, 0.085388994307400434, 1.0);
        }
        if (val <= 0.628906)
        {
            return Colour::fromFloatRGBA(0.89500316255534462, 1.0, 0.072738772928526263, 1.0);
        }
        if (val <= 0.632812)
        {
            return Colour::fromFloatRGBA(0.90765338393421868, 1.0, 0.060088551549652092, 1.0);
        }
        if (val <= 0.636719)
        {
            return Colour::fromFloatRGBA(0.92030360531309285, 1.0, 0.047438330170778031, 1.0);
        }
        if (val <= 0.640625)
        {
            return Colour::fromFloatRGBA(0.93295382669196703, 1.0, 0.03478810879190386, 1.0);
        }
        if (val <= 0.644531)
        {
            return Colour::fromFloatRGBA(0.94560404807084075, 0.98838053740014586, 0.022137887413030133, 1.0);
        }
        if (val <= 0.648438)
        {
            return Colour::fromFloatRGBA(0.95825426944971526, 0.973856209150327, 0.0094876660341556285, 1.0);
        }
        if (val <= 0.652344)
        {
            return Colour::fromFloatRGBA(0.97090449082858932, 0.95933188090050858, 0.0, 1.0);
        }
        if (val <= 0.656250)
        {
            return Colour::fromFloatRGBA(0.98355471220746349, 0.94480755265069016, 0.0, 1.0);
        }
        if (val <= 0.660156)
        {
            return Colour::fromFloatRGBA(0.99620493358633766, 0.93028322440087174, 0.0, 1.0);
        }
        if (val <= 0.664062)
        {
            return Colour::fromFloatRGBA(1.0, 0.91575889615105321, 0.0, 1.0);
        }
        if (val <= 0.667969)
        {
            return Colour::fromFloatRGBA(1.0, 0.9012345679012348, 0.0, 1.0);
        }
        if (val <= 0.671875)
        {
            return Colour::fromFloatRGBA(1.0, 0.88671023965141638, 0.0, 1.0);
        }
        if (val <= 0.675781)
        {
            return Colour::fromFloatRGBA(1.0, 0.87218591140159796, 0.0, 1.0);
        }
        if (val <= 0.679688)
        {
            return Colour::fromFloatRGBA(1.0, 0.85766158315177943, 0.0, 1.0);
        }
        if (val <= 0.683594)
        {
            return Colour::fromFloatRGBA(1.0, 0.84313725490196101, 0.0, 1.0);
        }
        if (val <= 0.687500)
        {
            return Colour::fromFloatRGBA(1.0, 0.82861292665214259, 0.0, 1.0);
        }
        if (val <= 0.691406)
        {
            return Colour::fromFloatRGBA(1.0, 0.81408859840232406, 0.0, 1.0);
        }
        if (val <= 0.695312)
        {
            return Colour::fromFloatRGBA(1.0, 0.79956427015250564, 0.0, 1.0);
        }
        if (val <= 0.699219)
        {
            return Colour::fromFloatRGBA(1.0, 0.78503994190268722, 0.0, 1.0);
        }
        if (val <= 0.703125)
        {
            return Colour::fromFloatRGBA(1.0, 0.7705156136528688, 0.0, 1.0);
        }
        if (val <= 0.707031)
        {
            return Colour::fromFloatRGBA(1.0, 0.75599128540305072, 0.0, 1.0);
        }
        if (val <= 0.710938)
        {
            return Colour::fromFloatRGBA(1.0, 0.74146695715323196, 0.0, 1.0);
        }
        if (val <= 0.714844)
        {
            return Colour::fromFloatRGBA(1.0, 0.72694262890341343, 0.0, 1.0);
        }
        if (val <= 0.718750)
        {
            return Colour::fromFloatRGBA(1.0, 0.71241830065359502, 0.0, 1.0);
        }
        if (val <= 0.722656)
        {
            return Colour::fromFloatRGBA(1.0, 0.69789397240377649, 0.0, 1.0);
        }
        if (val <= 0.726562)
        {
            return Colour::fromFloatRGBA(1.0, 0.68336964415395807, 0.0, 1.0);
        }
        if (val <= 0.730469)
        {
            return Colour::fromFloatRGBA(1.0, 0.66884531590413965, 0.0, 1.0);
        }
        if (val <= 0.734375)
        {
            return Colour::fromFloatRGBA(1.0, 0.65432098765432123, 0.0, 1.0);
        }
        if (val <= 0.738281)
        {
            return Colour::fromFloatRGBA(1.0, 0.63979665940450281, 0.0, 1.0);
        }
        if (val <= 0.742188)
        {
            return Colour::fromFloatRGBA(1.0, 0.62527233115468439, 0.0, 1.0);
        }
        if (val <= 0.746094)
        {
            return Colour::fromFloatRGBA(1.0, 0.61074800290486586, 0.0, 1.0);
        }
        if (val <= 0.750000)
        {
            return Colour::fromFloatRGBA(1.0, 0.59622367465504744, 0.0, 1.0);
        }
        if (val <= 0.753906)
        {
            return Colour::fromFloatRGBA(1.0, 0.58169934640522891, 0.0, 1.0);
        }
        if (val <= 0.757812)
        {
            return Colour::fromFloatRGBA(1.0, 0.56717501815541049, 0.0, 1.0);
        }
        if (val <= 0.761719)
        {
            return Colour::fromFloatRGBA(1.0, 0.55265068990559207, 0.0, 1.0);
        }
        if (val <= 0.765625)
        {
            return Colour::fromFloatRGBA(1.0, 0.53812636165577366, 0.0, 1.0);
        }
        if (val <= 0.769531)
        {
            return Colour::fromFloatRGBA(1.0, 0.52360203340595557, 0.0, 1.0);
        }
        if (val <= 0.773438)
        {
            return Colour::fromFloatRGBA(1.0, 0.50907770515613682, 0.0, 1.0);
        }
        if (val <= 0.777344)
        {
            return Colour::fromFloatRGBA(1.0, 0.49455337690631829, 0.0, 1.0);
        }
        if (val <= 0.781250)
        {
            return Colour::fromFloatRGBA(1.0, 0.48002904865649987, 0.0, 1.0);
        }
        if (val <= 0.785156)
        {
            return Colour::fromFloatRGBA(1.0, 0.46550472040668145, 0.0, 1.0);
        }
        if (val <= 0.789062)
        {
            return Colour::fromFloatRGBA(1.0, 0.45098039215686292, 0.0, 1.0);
        }
        if (val <= 0.792969)
        {
            return Colour::fromFloatRGBA(1.0, 0.4364560639070445, 0.0, 1.0);
        }
        if (val <= 0.796875)
        {
            return Colour::fromFloatRGBA(1.0, 0.42193173565722608, 0.0, 1.0);
        }
        if (val <= 0.800781)
        {
            return Colour::fromFloatRGBA(1.0, 0.40740740740740755, 0.0, 1.0);
        }
        if (val <= 0.804688)
        {
            return Colour::fromFloatRGBA(1.0, 0.39288307915758913, 0.0, 1.0);
        }
        if (val <= 0.808594)
        {
            return Colour::fromFloatRGBA(1.0, 0.37835875090777071, 0.0, 1.0);
        }
        if (val <= 0.812500)
        {
            return Colour::fromFloatRGBA(1.0, 0.36383442265795229, 0.0, 1.0);
        }
        if (val <= 0.816406)
        {
            return Colour::fromFloatRGBA(1.0, 0.34931009440813376, 0.0, 1.0);
        }
        if (val <= 0.820312)
        {
            return Colour::fromFloatRGBA(1.0, 0.33478576615831535, 0.0, 1.0);
        }
        if (val <= 0.824219)
        {
            return Colour::fromFloatRGBA(1.0, 0.32026143790849693, 0.0, 1.0);
        }
        if (val <= 0.828125)
        {
            return Colour::fromFloatRGBA(1.0, 0.30573710965867851, 0.0, 1.0);
        }
        if (val <= 0.832031)
        {
            return Colour::fromFloatRGBA(1.0, 0.29121278140886042, 0.0, 1.0);
        }
        if (val <= 0.835938)
        {
            return Colour::fromFloatRGBA(1.0, 0.27668845315904156, 0.0, 1.0);
        }
        if (val <= 0.839844)
        {
            return Colour::fromFloatRGBA(1.0, 0.26216412490922314, 0.0, 1.0);
        }
        if (val <= 0.843750)
        {
            return Colour::fromFloatRGBA(1.0, 0.24763979665940472, 0.0, 1.0);
        }
        if (val <= 0.847656)
        {
            return Colour::fromFloatRGBA(1.0, 0.23311546840958619, 0.0, 1.0);
        }
        if (val <= 0.851562)
        {
            return Colour::fromFloatRGBA(1.0, 0.21859114015976777, 0.0, 1.0);
        }
        if (val <= 0.855469)
        {
            return Colour::fromFloatRGBA(1.0, 0.20406681190994935, 0.0, 1.0);
        }
        if (val <= 0.859375)
        {
            return Colour::fromFloatRGBA(1.0, 0.18954248366013093, 0.0, 1.0);
        }
        if (val <= 0.863281)
        {
            return Colour::fromFloatRGBA(1.0, 0.1750181554103124, 0.0, 1.0);
        }
        if (val <= 0.867188)
        {
            return Colour::fromFloatRGBA(1.0, 0.16049382716049398, 0.0, 1.0);
        }
        if (val <= 0.871094)
        {
            return Colour::fromFloatRGBA(1.0, 0.14596949891067557, 0.0, 1.0);
        }
        if (val <= 0.875000)
        {
            return Colour::fromFloatRGBA(1.0, 0.13144517066085715, 0.0, 1.0);
        }
        if (val <= 0.878906)
        {
            return Colour::fromFloatRGBA(1.0, 0.11692084241103862, 0.0, 1.0);
        }
        if (val <= 0.882812)
        {
            return Colour::fromFloatRGBA(1.0, 0.1023965141612202, 0.0, 1.0);
        }
        if (val <= 0.886719)
        {
            return Colour::fromFloatRGBA(1.0, 0.087872185911401779, 0.0, 1.0);
        }
        if (val <= 0.890625)
        {
            return Colour::fromFloatRGBA(0.99910873440285231, 0.07334785766158336, 0.0, 1.0);
        }
        if (val <= 0.894531)
        {
            return Colour::fromFloatRGBA(0.98128342245989386, 0.058823529411765274, 0.0, 1.0);
        }
        if (val <= 0.898438)
        {
            return Colour::fromFloatRGBA(0.96345811051693431, 0.044299201161946411, 0.0, 1.0);
        }
        if (val <= 0.902344)
        {
            return Colour::fromFloatRGBA(0.94563279857397531, 0.029774872912127992, 0.0, 1.0);
        }
        if (val <= 0.906250)
        {
            return Colour::fromFloatRGBA(0.92780748663101631, 0.015250544662309573, 0.0, 1.0);
        }
        if (val <= 0.910156)
        {
            return Colour::fromFloatRGBA(0.90998217468805731, 0.00072621641249104307, 0.0, 1.0);
        }
        if (val <= 0.914062)
        {
            return Colour::fromFloatRGBA(0.89215686274509831, 0.0, 0.0, 1.0);
        }
        if (val <= 0.917969)
        {
            return Colour::fromFloatRGBA(0.8743315508021392, 0.0, 0.0, 1.0);
        }
        if (val <= 0.921875)
        {
            return Colour::fromFloatRGBA(0.8565062388591802, 0.0, 0.0, 1.0);
        }
        if (val <= 0.925781)
        {
            return Colour::fromFloatRGBA(0.8386809269162212, 0.0, 0.0, 1.0);
        }
        if (val <= 0.929688)
        {
            return Colour::fromFloatRGBA(0.8208556149732622, 0.0, 0.0, 1.0);
        }
        if (val <= 0.933594)
        {
            return Colour::fromFloatRGBA(0.80303030303030321, 0.0, 0.0, 1.0);
        }
        if (val <= 0.937500)
        {
            return Colour::fromFloatRGBA(0.78520499108734421, 0.0, 0.0, 1.0);
        }
        if (val <= 0.941406)
        {
            return Colour::fromFloatRGBA(0.76737967914438521, 0.0, 0.0, 1.0);
        }
        if (val <= 0.945312)
        {
            return Colour::fromFloatRGBA(0.74955436720142621, 0.0, 0.0, 1.0);
        }
        if (val <= 0.949219)
        {
            return Colour::fromFloatRGBA(0.73172905525846721, 0.0, 0.0, 1.0);
        }
        if (val <= 0.953125)
        {
            return Colour::fromFloatRGBA(0.71390374331550821, 0.0, 0.0, 1.0);
        }
        if (val <= 0.957031)
        {
            return Colour::fromFloatRGBA(0.69607843137254966, 0.0, 0.0, 1.0);
        }
        if (val <= 0.960938)
        {
            return Colour::fromFloatRGBA(0.6782531194295901, 0.0, 0.0, 1.0);
        }
        if (val <= 0.964844)
        {
            return Colour::fromFloatRGBA(0.6604278074866311, 0.0, 0.0, 1.0);
        }
        if (val <= 0.968750)
        {
            return Colour::fromFloatRGBA(0.6426024955436721, 0.0, 0.0, 1.0);
        }
        if (val <= 0.972656)
        {
            return Colour::fromFloatRGBA(0.6247771836007131, 0.0, 0.0, 1.0);
        }
        if (val <= 0.976562)
        {
            return Colour::fromFloatRGBA(0.60695187165775399, 0.0, 0.0, 1.0);
        }
        if (val <= 0.980469)
        {
            return Colour::fromFloatRGBA(0.589126559714795, 0.0, 0.0, 1.0);
        }
        if (val <= 0.984375)
        {
            return Colour::fromFloatRGBA(0.571301247771836, 0.0, 0.0, 1.0);
        }
        if (val <= 0.988281)
        {
            return Colour::fromFloatRGBA(0.553475935828877, 0.0, 0.0, 1.0);
        }
        if (val <= 0.992188)
        {
            return Colour::fromFloatRGBA(0.535650623885918, 0.0, 0.0, 1.0);
        }
        if (val <= 0.996094)
        {
            return Colour::fromFloatRGBA(0.517825311942959, 0.0, 0.0, 1.0);
        }

        return Colour::fromFloatRGBA(0.5, 0.0, 0.0, 1.0);
    }
} // namespace
