#include "UsgsAstroFramePlugin.h"
#include "UsgsAstroFrameSensorModel.h"

#include <json.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <gtest/gtest.h>

using namespace std;
using json = nlohmann::json;

class FrameIsdTest : public ::testing::Test {
   protected:

      csm::Isd isd;

   virtual void SetUp() {
      std::ifstream isdFile("data/simpleFramerISD.json");
      json jsonIsd = json::parse(isdFile);
      for (json::iterator it = jsonIsd.begin(); it != jsonIsd.end(); ++it) {
         json jsonValue = it.value();
         if (jsonValue.size() > 1) {
            for (int i = 0; i < jsonValue.size(); i++) {
               isd.addParam(it.key(), jsonValue[i].dump());
            }
         }
         else {
            isd.addParam(it.key(), jsonValue.dump());
         }
      }
      isdFile.close();
   }
};

class FrameSensorModel : public ::testing::Test {
   protected:


      protected :
      UsgsAstroFrameSensorModel *sensorModel;
      void loadIsd(const std::string pathtoIsd,csm::Isd &isd){
        std::ifstream isdFile("data/simpleFramerISD.json");
        json jsonIsd = json::parse(isdFile);

        for (json::iterator it = jsonIsd.begin(); it != jsonIsd.end(); ++it) {
           json jsonValue = it.value();
           if (jsonValue.size() > 1) {
             for (int i = 0; i < jsonValue.size(); i++) {
                   isd.addParam(it.key(), jsonValue[i].dump());
             }
           }
           else {
              isd.addParam(it.key(), jsonValue.dump());
           }
        }
      }


      void SetUp() override {
         sensorModel = NULL;
         std::ifstream isdFile("data/simpleFramerISD.json");
         json jsonIsd = json::parse(isdFile);
         csm::Isd isd;
         for (json::iterator it = jsonIsd.begin(); it != jsonIsd.end(); ++it) {
            json jsonValue = it.value();
            if (jsonValue.size() > 1) {
              for (int i = 0; i < jsonValue.size(); i++){
                    isd.addParam(it.key(), jsonValue[i].dump());
               }
            }
            else {
               isd.addParam(it.key(), jsonValue.dump());
            }
         }

         isdFile.close();

         UsgsAstroFramePlugin frameCameraPlugin;

         csm::Model *model = frameCameraPlugin.constructModelFromISD(
               isd,
               "USGS_ASTRO_FRAME_SENSOR_MODEL");

         sensorModel = dynamic_cast<UsgsAstroFrameSensorModel *>(model);

         ASSERT_NE(sensorModel, nullptr);
      }

      void TearDown() override {
         if (sensorModel) {
            delete sensorModel;
            sensorModel = NULL;
         }

      }
};

//NOTE: The imagePt format is (Lines,Samples)

//centered and slightly off-center:
TEST_F(FrameSensorModel, Center) {
   csm::ImageCoord imagePt(7.5, 7.5);
   csm::EcefCoord groundPt = sensorModel->imageToGround(imagePt, 0.0);
   EXPECT_NEAR(groundPt.x, 10.0, 1e-8);
   EXPECT_NEAR(groundPt.y, 0, 1e-8);
   EXPECT_NEAR(groundPt.z, 0, 1e-8);
}
TEST_F(FrameSensorModel, SlightlyOffCenter) {
   csm::ImageCoord imagePt(7.5, 6.5);
   csm::EcefCoord groundPt = sensorModel->imageToGround(imagePt, 0.0);
   EXPECT_NEAR(groundPt.x, 9.80194018, 1e-8);
   EXPECT_NEAR(groundPt.y, 0, 1e-8);
   EXPECT_NEAR(groundPt.z, 1.98039612, 1e-8);
}

//Test all four corners:
TEST_F(FrameSensorModel, OffBody1) {
   csm::ImageCoord imagePt(15.0, 0.0);
   csm::EcefCoord groundPt = sensorModel->imageToGround(imagePt, 0.0);
   EXPECT_NEAR(groundPt.x, 0.44979759, 1e-8);
   EXPECT_NEAR(groundPt.y, -14.99325304, 1e-8);
   EXPECT_NEAR(groundPt.z, 14.99325304, 1e-8);
}
TEST_F(FrameSensorModel, OffBody2) {
   csm::ImageCoord imagePt(0.0, 15.0);
   csm::EcefCoord groundPt = sensorModel->imageToGround(imagePt, 0.0);
   EXPECT_NEAR(groundPt.x, 0.44979759, 1e-8);
   EXPECT_NEAR(groundPt.y, 14.99325304, 1e-8);
   EXPECT_NEAR(groundPt.z, -14.99325304, 1e-8);
}
TEST_F(FrameSensorModel, OffBody3) {
   csm::ImageCoord imagePt(0.0, 0.0);
   csm::EcefCoord groundPt = sensorModel->imageToGround(imagePt, 0.0);
   EXPECT_NEAR(groundPt.x, 0.44979759, 1e-8);
   EXPECT_NEAR(groundPt.y, 14.99325304, 1e-8);
   EXPECT_NEAR(groundPt.z, 14.99325304, 1e-8);
}
TEST_F(FrameSensorModel, OffBody4) {
   csm::ImageCoord imagePt(15.0, 15.0);
   csm::EcefCoord groundPt = sensorModel->imageToGround(imagePt, 0.0);
   EXPECT_NEAR(groundPt.x, 0.44979759, 1e-8);
   EXPECT_NEAR(groundPt.y, -14.99325304, 1e-8);
   EXPECT_NEAR(groundPt.z, -14.99325304, 1e-8);
}


TEST_F(FrameSensorModel, setFocalPlane1) {
  int precision  = 20;

   csm::Isd localIsd;
   loadIsd("data/simpleFramerISD.json",localIsd);
   csm::ImageCoord imagePt(7.0, 7.0);
   double ux,uy;
   vector<double> odtx{0.0,1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
   vector<double> odty{0.0,1.0,1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

   localIsd.clearParams("odt_x");
   localIsd.clearParams("odt_y");
   for (auto & val: odtx){
      ostringstream strval;
      strval << setprecision(precision) << val;
      localIsd.addParam("odt_x", strval.str());
   }
   for (auto & val: odty){
      ostringstream strval;
      strval << setprecision(precision) << val;
      localIsd.addParam("odt_y", strval.str());
   }

   sensorModel->setFocalPlane(imagePt.samp, imagePt.line, ux, uy);
   EXPECT_NEAR(ux,7.0,1e-8 );
   EXPECT_NEAR(uy,7.0,1e-8);

}



// Focal Length Tests:
TEST_F(FrameIsdTest, FL500_OffBody4) {
   std::string key = "focal_length";
   std::string newValue = "500.0";
   isd.clearParams(key);
   isd.addParam(key,newValue);
   UsgsAstroFramePlugin frameCameraPlugin;

   csm::Model *model = frameCameraPlugin.constructModelFromISD(
         isd,
         "USGS_ASTRO_FRAME_SENSOR_MODEL");

   UsgsAstroFrameSensorModel* sensorModel = dynamic_cast<UsgsAstroFrameSensorModel *>(model);

   ASSERT_NE(sensorModel, nullptr);
   csm::ImageCoord imagePt(15.0, 15.0);
   csm::EcefCoord groundPt = sensorModel->imageToGround(imagePt, 0.0);
   EXPECT_NEAR(groundPt.x, 9.77688917, 1e-8);
   EXPECT_NEAR(groundPt.y, -1.48533467, 1e-8);
   EXPECT_NEAR(groundPt.z, -1.48533467, 1e-8);
}
TEST_F(FrameIsdTest, FL500_OffBody3) {
   std::string key = "focal_length";
   std::string newValue = "500.0";
   isd.clearParams(key);
   isd.addParam(key,newValue);
   UsgsAstroFramePlugin frameCameraPlugin;

   csm::Model *model = frameCameraPlugin.constructModelFromISD(
         isd,
         "USGS_ASTRO_FRAME_SENSOR_MODEL");

   UsgsAstroFrameSensorModel* sensorModel = dynamic_cast<UsgsAstroFrameSensorModel *>(model);

   ASSERT_NE(sensorModel, nullptr);
   csm::ImageCoord imagePt(0.0, 0.0);
   csm::EcefCoord groundPt = sensorModel->imageToGround(imagePt, 0.0);
   EXPECT_NEAR(groundPt.x, 9.77688917, 1e-8);
   EXPECT_NEAR(groundPt.y, 1.48533467, 1e-8);
   EXPECT_NEAR(groundPt.z, 1.48533467, 1e-8);
}
TEST_F(FrameIsdTest, FL500_Center) {
   std::string key = "focal_length";
   std::string newValue = "500.0";
   isd.clearParams(key);
   isd.addParam(key,newValue);
   UsgsAstroFramePlugin frameCameraPlugin;

   csm::Model *model = frameCameraPlugin.constructModelFromISD(
         isd,
         "USGS_ASTRO_FRAME_SENSOR_MODEL");

   UsgsAstroFrameSensorModel* sensorModel = dynamic_cast<UsgsAstroFrameSensorModel *>(model);

   ASSERT_NE(sensorModel, nullptr);
   csm::ImageCoord imagePt(7.5, 7.5);
   csm::EcefCoord groundPt = sensorModel->imageToGround(imagePt, 0.0);
   EXPECT_NEAR(groundPt.x, 10.0, 1e-8);
   EXPECT_NEAR(groundPt.y, 0.0, 1e-8);
   EXPECT_NEAR(groundPt.z, 0.0, 1e-8);
}
TEST_F(FrameIsdTest, FL500_SlightlyOffCenter) {
   std::string key = "focal_length";
   std::string newValue = "500.0";
   isd.clearParams(key);
   isd.addParam(key,newValue);
   UsgsAstroFramePlugin frameCameraPlugin;

   csm::Model *model = frameCameraPlugin.constructModelFromISD(
         isd,
         "USGS_ASTRO_FRAME_SENSOR_MODEL");

   UsgsAstroFrameSensorModel* sensorModel = dynamic_cast<UsgsAstroFrameSensorModel *>(model);

   ASSERT_NE(sensorModel, nullptr);
   csm::ImageCoord imagePt(7.5, 6.5);
   csm::EcefCoord groundPt = sensorModel->imageToGround(imagePt, 0.0);
   EXPECT_NEAR(groundPt.x, 9.99803960, 1e-8);
   EXPECT_NEAR(groundPt.y, 0.0, 1e-8);
   EXPECT_NEAR(groundPt.z, 1.98000392e-01, 1e-8);

}

//Observer x position:
TEST_F(FrameIsdTest, X10_SlightlyOffCenter) {
   std::string key = "x_sensor_origin";
   std::string newValue = "10.0";
   isd.clearParams(key);
   isd.addParam(key,newValue);
   UsgsAstroFramePlugin frameCameraPlugin;

   csm::Model *model = frameCameraPlugin.constructModelFromISD(
         isd,
         "USGS_ASTRO_FRAME_SENSOR_MODEL");

   UsgsAstroFrameSensorModel* sensorModel = dynamic_cast<UsgsAstroFrameSensorModel *>(model);

   ASSERT_NE(sensorModel, nullptr);
   csm::ImageCoord imagePt(7.5, 6.5);
   csm::EcefCoord groundPt = sensorModel->imageToGround(imagePt, 0.0);
   EXPECT_NEAR(groundPt.x, 10.0, 1e-8);
   EXPECT_NEAR(groundPt.y, 0.0, 1e-8);
   EXPECT_NEAR(groundPt.z, 0.0, 1e-8);

}
TEST_F(FrameIsdTest, X1e9_SlightlyOffCenter) {
   std::string key = "x_sensor_origin";
   std::string newValue = "1000000000.0";
   isd.clearParams(key);
   isd.addParam(key,newValue);
   UsgsAstroFramePlugin frameCameraPlugin;

   csm::Model *model = frameCameraPlugin.constructModelFromISD(
         isd,
         "USGS_ASTRO_FRAME_SENSOR_MODEL");

   UsgsAstroFrameSensorModel* sensorModel = dynamic_cast<UsgsAstroFrameSensorModel *>(model);

   ASSERT_NE(sensorModel, nullptr);
   csm::ImageCoord imagePt(7.5, 6.5);
   csm::EcefCoord groundPt = sensorModel->imageToGround(imagePt, 0.0);
   //Note: In the following, the tolerance was increased due to the very large distance being tested (~6.68 AU).
   EXPECT_NEAR(groundPt.x, 3.99998400e+03, 1e-4);
   EXPECT_NEAR(groundPt.y, 0.0, 1e-4);
   EXPECT_NEAR(groundPt.z, 1.99999200e+06, 1e-4);

}
