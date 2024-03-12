#include "ccmwindow.hpp"
#ifdef WITH_CCM
#include <opencv2/mcc.hpp>
#endif
#include "draw.hpp"

#define CTL_SPACE       "Color Space"
#define CTL_DISTANCE    "Distance Type"
#define CTL_TYPE        "Linear Type"
#define CTL_GAMMA       "Linear Gamma"
#define CTL_DEGREE      "Linear Degree"
#define CTL_LOWER_SAT   "Lower Saturation"
#define CTL_UPPER_SAT   "Upper Saturation"
#define BTN_ACTIVE      "Active"
#define BTN_COLORS      "Show colors"


CCMWindow::CCMWindow(const char *name) :
        Window(name)
{
#ifdef WITH_CCM
        DescriptionMap spaces {
                { ccm::COLOR_SPACE_sRGB,               "COLOR_SPACE_sRGB                " },
                // { ccm::COLOR_SPACE_sRGBL,              "COLOR_SPACE_sRGBL               " },
                { ccm::COLOR_SPACE_AdobeRGB,           "COLOR_SPACE_AdobeRGB            " },
                // { ccm::COLOR_SPACE_AdobeRGBL,          "COLOR_SPACE_AdobeRGBL           " },
                { ccm::COLOR_SPACE_WideGamutRGB,       "COLOR_SPACE_WideGamutRGB        " },
                // { ccm::COLOR_SPACE_WideGamutRGBL,      "COLOR_SPACE_WideGamutRGBL       " },
                { ccm::COLOR_SPACE_ProPhotoRGB,        "COLOR_SPACE_ProPhotoRGB         " },
                // { ccm::COLOR_SPACE_ProPhotoRGBL,       "COLOR_SPACE_ProPhotoRGBL        " },
                { ccm::COLOR_SPACE_DCI_P3_RGB,         "COLOR_SPACE_DCI_P3_RGB          " },
                // { ccm::COLOR_SPACE_DCI_P3_RGBL,        "COLOR_SPACE_DCI_P3_RGBL         " },
                { ccm::COLOR_SPACE_AppleRGB,           "COLOR_SPACE_AppleRGB            " },
                // { ccm::COLOR_SPACE_AppleRGBL,          "COLOR_SPACE_AppleRGBL           " },
                { ccm::COLOR_SPACE_REC_709_RGB,        "COLOR_SPACE_REC_709_RGB         " },
                // { ccm::COLOR_SPACE_REC_709_RGBL,       "COLOR_SPACE_REC_709_RGBL        " },
                { ccm::COLOR_SPACE_REC_2020_RGB,       "COLOR_SPACE_REC_2020_RGB        " }
                // { ccm::COLOR_SPACE_REC_2020_RGBL,      "COLOR_SPACE_REC_2020_RGBL       " },
                // { ccm::COLOR_SPACE_XYZ_D65_2,          "COLOR_SPACE_XYZ_D65_2           " },
                // { ccm::COLOR_SPACE_XYZ_D65_10,         "COLOR_SPACE_XYZ_D65_10          " },
                // { ccm::COLOR_SPACE_XYZ_D50_2,          "COLOR_SPACE_XYZ_D50_2           " },
                // { ccm::COLOR_SPACE_XYZ_D50_10,         "COLOR_SPACE_XYZ_D50_10          " },
                // { ccm::COLOR_SPACE_XYZ_A_2,            "COLOR_SPACE_XYZ_A_2             " },
                // { ccm::COLOR_SPACE_XYZ_A_10,           "COLOR_SPACE_XYZ_A_10            " },
                // { ccm::COLOR_SPACE_XYZ_D55_2,          "COLOR_SPACE_XYZ_D55_2           " },
                // { ccm::COLOR_SPACE_XYZ_D55_10,         "COLOR_SPACE_XYZ_D55_10          " },
                // { ccm::COLOR_SPACE_XYZ_D75_2,          "COLOR_SPACE_XYZ_D75_2           " },
                // { ccm::COLOR_SPACE_XYZ_D75_10,         "COLOR_SPACE_XYZ_D75_10          " },
                // { ccm::COLOR_SPACE_XYZ_E_2,            "COLOR_SPACE_XYZ_E_2             " },
                // { ccm::COLOR_SPACE_XYZ_E_10,           "COLOR_SPACE_XYZ_E_10            " },
                // { ccm::COLOR_SPACE_Lab_D65_2,          "COLOR_SPACE_Lab_D65_2           " },
                // { ccm::COLOR_SPACE_Lab_D65_10,         "COLOR_SPACE_Lab_D65_10          " },
                // { ccm::COLOR_SPACE_Lab_D50_2,          "COLOR_SPACE_Lab_D50_2           " },
                // { ccm::COLOR_SPACE_Lab_D50_10,         "COLOR_SPACE_Lab_D50_10          " },
                // { ccm::COLOR_SPACE_Lab_A_2,            "COLOR_SPACE_Lab_A_2             " },
                // { ccm::COLOR_SPACE_Lab_A_10,           "COLOR_SPACE_Lab_A_10            " },
                // { ccm::COLOR_SPACE_Lab_D55_2,          "COLOR_SPACE_Lab_D55_2           " },
                // { ccm::COLOR_SPACE_Lab_D55_10,         "COLOR_SPACE_Lab_D55_10          " },
                // { ccm::COLOR_SPACE_Lab_D75_2,          "COLOR_SPACE_Lab_D75_2           " },
                // { ccm::COLOR_SPACE_Lab_D75_10,         "COLOR_SPACE_Lab_D75_10          " },
                // { ccm::COLOR_SPACE_Lab_E_2,            "COLOR_SPACE_Lab_E_2             " },
                // { ccm::COLOR_SPACE_Lab_E_10,           "COLOR_SPACE_Lab_E_10            " }
        };
        addControl(new Control(CTL_SPACE,     "", 0,    0,    1,    0,   38, 1, 3, spaces));

        DescriptionMap distances {
                { ccm::DISTANCE_CIE76,                  "DISTANCE_CIE76                 " },
                { ccm::DISTANCE_CIE94_GRAPHIC_ARTS,     "DISTANCE_CIE94_GRAPHIC_ARTS    " },
                { ccm::DISTANCE_CIE94_TEXTILES,         "DISTANCE_CIE94_TEXTILES        " },
                { ccm::DISTANCE_CIE2000,                "DISTANCE_CIE2000               " },
                { ccm::DISTANCE_CMC_1TO1,               "DISTANCE_CMC_1TO1              " },
                { ccm::DISTANCE_CMC_2TO1,               "DISTANCE_CMC_2TO1              " },
                { ccm::DISTANCE_RGB,                    "DISTANCE_RGB                   " },
                { ccm::DISTANCE_RGBL,                   "DISTANCE_RGBL                  " }
        };
        addControl(new Control(CTL_DISTANCE,  "", 0,    3,    1,    0,    7, 1, 4, distances));

        DescriptionMap types {
                { ccm::LINEARIZATION_IDENTITY,          "LINEARIZATION_IDENTITY         " },
                { ccm::LINEARIZATION_GAMMA,             "LINEARIZATION_GAMMA            " },
                { ccm::LINEARIZATION_COLORPOLYFIT,      "LINEARIZATION_COLORPOLYFIT     " },
                { ccm::LINEARIZATION_COLORLOGPOLYFIT,   "LINEARIZATION_COLORLOGPOLYFIT  " },
                { ccm::LINEARIZATION_GRAYPOLYFIT,       "LINEARIZATION_GRAYPOLYFIT      " },
                { ccm::LINEARIZATION_GRAYLOGPOLYFIT,    "LINEARIZATION_GRAYLOGPOLYFIT   " }
        };
        addControl(new Control(CTL_TYPE,      "", 0,    1,    1,    0,    5, 1, 5, types));

        addControl(new Control(CTL_GAMMA,     "", 1,  2.2,  0.1,  0.1,  5.0, 1, 6));
        addControl(new Control(CTL_DEGREE,    "", 0,    3,    1,    2,    5, 1, 7));
        addControl(new Control(CTL_LOWER_SAT, "", 2, 0.00, 0.01, 0.00, 2.00, 1, 8));
        addControl(new Control(CTL_UPPER_SAT, "", 2, 0.98, 0.01, 0.01, 2.00, 1, 9));      

        addButton(new Button(BTN_ACTIVE, -1, 1));
        addButton(new Button(BTN_COLORS, -1, 2));
#endif
}

void CCMWindow::show(Mat img)
{
        Button *activeButton = button(BTN_ACTIVE);
        assert(activeButton != NULL);

        if (img.channels() != 3 || !activeButton->pressed()) {
                Window::show(img);
                return;
        }

        Mat imageBGR = img;
        Mat imageShow = imageBGR;

#ifdef WITH_CCM

        Ptr<mcc::CCheckerDetector> detector = mcc::CCheckerDetector::create();
        if (detector->process(imageBGR, mcc::MCC24)) {
                std::vector<Ptr<mcc::CChecker>> checkers = detector->getListColorChecker();
                for (Ptr<mcc::CChecker> checker : checkers) {
                        Mat chartsRGB = checker->getChartsRGB();
                        Mat detectedColors = chartsRGB.col(1).clone().reshape(3, chartsRGB.rows/3);
                        detectedColors /= 255.0;

                        ccm::ColorCorrectionModel model(detectedColors, ccm::COLORCHECKER_Macbeth);
                        model.setCCM_TYPE(ccm::CCM_3x3);
                        model.setColorSpace(ccm::COLOR_SPACE(control(CTL_SPACE)->value()));
                        model.setDistance(ccm::DISTANCE_TYPE(control(CTL_DISTANCE)->value()));
                        model.setLinear((ccm::LINEAR_TYPE(control(CTL_TYPE)->value())));
                        model.setLinearGamma(control(CTL_GAMMA)->value());
                        model.setLinearDegree(control(CTL_DEGREE)->value());
                        model.setSaturatedThreshold(
                                control(CTL_LOWER_SAT)->value(), 
                                control(CTL_UPPER_SAT)->value());
                        try {
                                model.run();

                        } catch(cv::Exception()) {

                        }

                        // Convert BGR image to normalized float image
                        Mat imageRGB;
                        cvtColor(imageBGR, imageRGB, COLOR_BGR2RGB);
                        Mat imageRGB_64F;
                        imageRGB.convertTo(imageRGB_64F, CV_64F);
                        Mat imageRGB_64F_norm = imageRGB_64F / 255;

                        // Apply color correction
                        Mat imageRGB_64F_norm_calib = model.infer(imageRGB_64F_norm);

                        // Convert normalized
                        Mat imageRGB_64F_calib = imageRGB_64F_norm_calib * 255;
                        Mat imageRGB_8UC3_calib;
                        imageRGB_64F_calib.convertTo(imageRGB_8UC3_calib, CV_8UC3);
                        // Mat imageRGB_calib = min(max(imageRGB_8UC3_calib, 0), 255);
                        Mat imageRGB_calib = imageRGB_8UC3_calib;
                        Mat imageBGR_calib;
                        cvtColor(imageRGB_calib, imageBGR_calib, COLOR_RGB2BGR);

                        Ptr<mcc::CCheckerDraw> checkerDraw = mcc::CCheckerDraw::create(checker);
                        checkerDraw->draw(imageBGR_calib);

                        Button *colorsButton = button(BTN_COLORS);
                        assert(colorsButton != NULL);
                        if (colorsButton->pressed()) {
                                Mat channels[3];
                                split(detectedColors, channels);
                                drawMat<double>(imageBGR_calib, channels[0],  -15, 3);
                                drawMat<double>(imageBGR_calib, channels[1],   -8, 3);
                                drawMat<double>(imageBGR_calib, channels[2],   -1, 3);
                        }

                        drawMat<double>(imageBGR_calib, model.getCCM(), 1, 11);

                        imageShow = imageBGR_calib;
                }
        }
#endif

        Window::show(imageShow);
}

// NOTE: Alternative implemetation to apply color correction matrix
//
// Mat imageRGB8Lin = imageRGB8.reshape(1, imageRGB8.cols*imageRGB8.rows);
// Mat imageCorrectedLin = imageRGB8Lin * ccm;
// imageRGB8 = imageCorrectedLin.reshape(3, image->height());