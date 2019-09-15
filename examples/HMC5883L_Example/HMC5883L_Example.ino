/*****************************************************************************/
// 機能：X軸の地理的方向を取得します。
// X軸が北を指す場合、0度です。
// X軸が東を指す場合、90度です。
// X軸が南を指す場合、180度です。
// X軸が西を指す場合、270度です。
//  Hardware:   Grove - 3-Axis Digital Compass
//	Arduino IDE: Arduino-1.0
//	Author:	 Frankie.Chu
//	Date: 	 Jan 10,2013
//	Version: v1.0
//
//  Modified by: Yihui Xiong
//  Data:        June 19, 2013
//  Description: add calibrate function
//
//	by www.seeedstudio.com
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
/*******************************************************************************/

// I2C Libraryを参照する
#include <Wire.h>
// HMC5883Lコンパスライブラリを参照する
#include <HMC5883L.h>

//#include "Streaming.h"

// コンパスを変数として保存します。
HMC5883L compass;
//コンパスで発生する可能性のあるエラーを記録します。
int error = 0;

MagnetometerScaled valueOffset;

//アウトセットアップルーチン、ここではマイクロコントローラーとコンパスを構成します。
void setup()
{
  //シリアルポートを初期化します。
  Serial.begin(115200);

  delay(2000);
  Serial.println("Starting the I2C interface.");
  Wire.begin(); // Start the I2C interface.

  Serial.println("Constructing new HMC5883L");

  Serial.println("Setting scale to +/- 1.3 Ga");
  error = compass.setScale(1.3); // Set the scale of the compass.
  if (error != 0)                // If there is an error, print it out.
    Serial.println(compass.getErrorText(error));

  Serial.println("Setting measurement mode to continous.");
  error = compass.setMeasurementMode(MEASUREMENT_CONTINUOUS); // Set the measurement mode to Continuous
  if (error != 0)                                             // If there is an error, print it out.
    Serial.println(compass.getErrorText(error));

  compassCalibrate();
}

// x、y、zのオフセットを調整します
void compassCalibrate(void)
{
  Serial.println("calibrate the compass");
  MagnetometerScaled valueMax = {0, 0, 0};
  MagnetometerScaled valueMin = {0, 0, 0};

  // x、y、zオフセットを計算します

  Serial.println("please rotate the compass");
  int xcount = 0;
  int ycount = 0;
  int zcount = 0;
  boolean xZero = false;
  boolean yZero = false;
  boolean zZero = false;
  MagnetometerScaled value;
  while (xcount < 3 || ycount < 3 || zcount < 3)
  {
    value = compass.readScaledAxis();
    if ((fabs(value.XAxis) > 600) || (fabs(value.YAxis) > 600) || (fabs(value.ZAxis) > 600))
    {
      continue;
    }

    if (valueMin.XAxis > value.XAxis)
    {
      valueMin.XAxis = value.XAxis;
    }
    else if (valueMax.XAxis < value.XAxis)
    {
      valueMax.XAxis = value.XAxis;
    }

    if (valueMin.YAxis > value.YAxis)
    {
      valueMin.YAxis = value.YAxis;
    }
    else if (valueMax.YAxis < value.YAxis)
    {
      valueMax.YAxis = value.YAxis;
    }

    if (valueMin.ZAxis > value.ZAxis)
    {
      valueMin.ZAxis = value.ZAxis;
    }
    else if (valueMax.ZAxis < value.ZAxis)
    {
      valueMax.ZAxis = value.ZAxis;
    }

    if (xZero)
    {
      if (fabs(value.XAxis) > 50)
      {
        xZero = false;
        xcount++;
      }
    }
    else
    {
      if (fabs(value.XAxis) < 40)
      {
        xZero = true;
      }
    }

    if (yZero)
    {
      if (fabs(value.YAxis) > 50)
      {
        yZero = false;
        ycount++;
      }
    }
    else
    {
      if (fabs(value.YAxis) < 40)
      {
        yZero = true;
      }
    }

    if (zZero)
    {
      if (fabs(value.ZAxis) > 50)
      {
        zZero = false;
        zcount++;
      }
    }
    else
    {
      if (fabs(value.ZAxis) < 40)
      {
        zZero = true;
      }
    }

    delay(30);
  }

  valueOffset.XAxis = (valueMax.XAxis + valueMin.XAxis) / 2;
  valueOffset.YAxis = (valueMax.YAxis + valueMin.YAxis) / 2;
  valueOffset.ZAxis = (valueMax.ZAxis + valueMin.ZAxis) / 2;
#if 0 
  Serial << "max: " << valueMax.XAxis << '\t' << valueMax.YAxis << '\t' << valueMax.ZAxis << endl;
  Serial << "min: " << valueMin.XAxis << '\t' << valueMin.YAxis << '\t' << valueMin.ZAxis << endl;
  Serial << "offset: " << valueOffset.XAxis << '\t' << valueOffset.YAxis << '\t' << valueOffset.ZAxis << endl;
  
  Serial << "<<<<" << endl;
#endif
  Serial.print("max: ");
  Serial.print(valueMax.XAxis);
  Serial.print(valueMax.YAxis);
  Serial.println(valueMax.ZAxis);
  Serial.print("min: ");
  Serial.print(valueMin.XAxis);
  Serial.print(valueMin.YAxis);
  Serial.println(valueMin.ZAxis);
  Serial.print("offset: ");
  Serial.print(valueOffset.XAxis);
  Serial.print(valueOffset.YAxis);
  Serial.println(valueOffset.ZAxis);
}

// メインプログラムループ。
void loop()
{
  //コンパスから生の値を取得します（スケーリングされません）。
  MagnetometerRaw raw = compass.readRawAxis();

  //コンパスからスケーリングされた値を取得しました（構成されたスケールにスケーリングされます）。
  MagnetometerScaled scaled = compass.readScaledAxis();

  scaled.XAxis -= valueOffset.XAxis;
  scaled.YAxis -= valueOffset.YAxis;
  scaled.ZAxis -= valueOffset.ZAxis;

  //値は次のようにアクセスされます：
  int MilliGauss_OnThe_XAxis = scaled.XAxis; // (or YAxis, or ZAxis)

  //磁力計が水平になったら方位を計算し、軸の兆候を修正します。
  float yxHeading = atan2(scaled.YAxis, scaled.XAxis);
  float zxHeading = atan2(scaled.ZAxis, scaled.XAxis);

  float heading = yxHeading;

  //見出しを取得したら、「偏角」を追加する必要があります。「偏角」は、現在地の磁場の「誤差」です。
     //ここで見つけてください：http://www.magnetic-declination.com/
   //私の場合：-2��37 '-2.617度、または（必要な）-0.0456752665ラジアン、-0.0457を使用します

      // Latitude: 35° 37' 28.9" N
      // Longitude : 139° 37' 54.4" E
      // Magnetic Declination: -7° 35'


   //赤緯が見つからない場合は、これら2行をコメントアウトすると、コンパスが少し外れます。
      float declinationAngle = -0.0457;
  heading += declinationAngle;

  //符号が逆になっている場合に修正します。
  if (heading < 0)
    heading += 2 * PI;

  // 赤緯の追加によるラップを確認します。
  if (heading > 2 * PI)
    heading -= 2 * PI;

  //読みやすくするためにラジアンを度に変換します。

  /*
  ラジアン（英: radian, 記号: rad）は、国際単位系 (SI) における角度（平面角）の単位である。 
  円周上でその円の半径と同じ長さの弧を切り取る2本の半径が成す角の値と定義される。 
  弧度（こど）とも言い、平面角の大きさをラジアンで測ることを弧度法と呼ぶ。
   */
  float headingDegrees = heading * 180 / M_PI;

  float yxHeadingDegrees = yxHeading * 180 / M_PI;
  float zxHeadingDegrees = zxHeading * 180 / M_PI;

  //シリアルポート経由でデータを出力します。
  // Output(raw, scaled, heading, headingDegrees);

  //  Serial << scaled.XAxis << ' ' << scaled.YAxis << ' ' << scaled.ZAxis << endl;
  //  Serial << "arctan y/x: " << yxHeadingDegrees << " \tarctan z/x: " << zxHeadingDegrees << endl;

  Serial.print(scaled.XAxis);
  Serial.print(scaled.YAxis);
  Serial.println(scaled.ZAxis);

  Serial.print("arctan y/x: ");
  Serial.print(yxHeadingDegrees);
  Serial.print("arctan z/x: ");
  Serial.print(zxHeadingDegrees);

  // 通常、ループを許可するためにアプリケーションを66ms遅延させます
  // 15Hzで実行します（HMC5883Lのデフォルト帯域幅）。
     // ただし、長いシリアルアウト（9600で104ミリ秒）があるため、
                  //自然な速度で実行されます。
      delay(1000); //　もちろん、より長く遅延させることができます。
}

//データをシリアルポートに出力します。
void Output(MagnetometerRaw raw, MagnetometerScaled scaled, float heading, float headingDegrees)
{
  Serial.print("Raw:\t");
  Serial.print(raw.XAxis);
  Serial.print("   ");
  Serial.print(raw.YAxis);
  Serial.print("   ");
  Serial.print(raw.ZAxis);
  Serial.print("   \tScaled:\t");

  Serial.print(scaled.XAxis);
  Serial.print("   ");
  Serial.print(scaled.YAxis);
  Serial.print("   ");
  Serial.print(scaled.ZAxis);

  Serial.print("   \tHeading:\t");
  Serial.print(heading);
  Serial.print(" Radians   \t");
  Serial.print(headingDegrees);
  Serial.println(" Degrees   \t");
}
