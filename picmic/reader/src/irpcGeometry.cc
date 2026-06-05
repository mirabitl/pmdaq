#include "irpcGeometry.hh"
//#include "utils.hh"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

using namespace lmana;
using namespace std;


lmana::irpcGeometry::irpcGeometry()
{
  
}

void lmana::irpcGeometry::initialize(std::string type, std::string side)
{
  // Xtof to connector (Top of strip to connector from center to extern)
  double clc31[48] = {90.543564, 80.410461, 70.289078, 60.173553, 50.069733, 40.903408, 36.713425, 32.523438, 31.166557, 36.356541, 40.546543, 49.202293, 59.317810, 69.433334, 79.548859, 89.664398, 56.088192, 66.203720, 76.319244, 86.428909, 96.550293, 106.665825, 116.775482, 126.896873, 137.006546, 147.122055, 157.243454, 167.358978, 177.474487, 187.590027, 197.699677, 207.821075, 175.118668, 185.379196, 195.481796, 205.597305, 215.848694, 225.834229, 235.949753, 246.065292, 256.180817, 266.302185, 276.671875, 286.533234, 301.558472, 314.650421, 328.493317, 343.113739};
  double clc41[48] = {90.894295, 80.668900, 70.553459, 60.438061, 50.316757, 40.429695, 36.239754, 0.890015,
                      31.640133, 34.830097, 39.020042, 44.321323, 57.034809, 68.150246, 77.265640, 79.041321,
                      56.276508, 64.931931, 75.047348, 85.162766, 95.278206, 105.393593, 115.509033, 125.624458, 135.739899, 145.855316, 155.970718, 166.086151, 143.920959, 187.317017, 141.053879, 207.547836, 0.880005, 184.098709, 194.214096, 204.329529, 214.444931, 224.560394, 234.675842, 244.791199, 10.737075, 265.022095, 275.137512, 285.252930, 300.251160, 313.273132, 327.085388, 340.790131};
  // Xtof Return (Low radius strip to connector)
  double clr31[48] = {2313.407715, 2303.935059, 2294.468262, 2285.000977, 2275.528320, 2266.067383, 2256.606445, 2247.133789, 2238.666504, 2228.200195, 2218.733398, 2209.266113, 2199.787598, 2190.332520, 2180.877441, 2171.387207, 2120.251953, 2110.785156, 2101.318359, 2091.851074, 2082.384277, 2072.917480, 2063.444824, 2053.983887, 2044.516846, 2035.049927, 2025.583008, 2016.116333, 2006.649414, 1997.182495, 1987.721558, 1978.243042, 1926.101929, 1916.629150, 1907.162231, 1897.701416, 1888.234497, 1878.773438, 1869.300659, 1855.762939, 1844.198853, 1832.120117, 1819.977539, 1807.770386, 1795.510132, 1783.159668, 1770.754272, 1757.014038};
  double clr41[48] = {2105.610596, 2095.231201, 2084.851562, 2074.472168, 2064.092773, 2053.713379, 2043.333496, 2064.113770,
                      2021.574585, 2012.194946, 2001.815796, 1994.034180, 1981.056641, 1969.677124, 1960.297363, 1958.257812,
                      1895.858643, 1886.479004, 1876.099609, 1865.719971, 1855.340576, 1844.960938, 1834.581787, 1824.202026, 1813.822510, 1803.442993, 1793.063354, 1782.683838, 1804.585083, 1760.925049, 1807.088135, 1740.165894, 1861.209717, 1677.726807, 1667.347534, 1656.968018, 1646.588623, 1636.208740, 1625.829346, 1615.449707, 1845.542236, 1588.633545, 1575.151978, 1561.595825, 1547.964844, 1534.258301, 1520.645142, 1505.342773};
  // Xtof strip length
  double cls31[48] = {1468.002441, 1468.022339, 1468.062012, 1468.121460, 1468.200928, 1468.300049, 1468.419067, 1468.557861, 1467.717041, 1468.895020, 1469.093262, 1469.311279, 1469.549194, 1469.806763, 1470.084229, 1470.381348, 1470.698242, 1471.034790, 1471.391235, 1471.767212, 1472.162964, 1472.578247, 1473.013306, 1473.468018, 1473.942383, 1474.436279, 1474.949707, 1475.482788, 1476.035400, 1476.607544, 1477.199097, 1477.810181, 1477.132324, 1479.090576, 1479.760010, 1480.448730, 1480.165649, 1481.884155, 1482.630737, 1481.655518, 1477.538940, 1472.481323, 1466.406738, 1462.278442, 1452.549316, 1436.809814, 1420.182617, 1403.590942};
  double cls41[48] = {1227.423828, 1227.126099, 1220.051514, 1220.100952, 1220.166870, 1220.249268, 1220.129150, 1220.048218,
                      1220.830566, 1220.743408, 1220.908203, 1220.385437, 1220.453369, 1221.862793, 1221.177856, 1221.770844, 1221.770844, 1222.520996, 1222.817017, 1223.129395, 1223.458008, 1223.803101, 1224.164551, 1224.542236, 1224.936279, 1225.346436, 1225.773071, 1226.215820, 1225.540161, 1227.914307, 1226.424438, 1228.963013, 1228.963013, 1229.212769, 1229.768799, 1230.340942, 1230.929077, 1231.533203, 1232.153442, 1232.789673, 1234.478271, 1225.268066, 1220.836914, 1214.699097, 1203.984131, 1187.263916, 1179.133911, 1151.987427};

  // Calculate position inside the strip
  /*
     t1 is T High radius
     t0 is T Low radius

     lc: distance High radius of strip to connector
     ls : strip length
     lr : distance Low radius of strip to connector
     z : distance along the strip from High radius

     t1= (lc+z)/V
     t0= (ls-z+lr)/V

     => V(t1-t0)=lc+2z -ls -lr

     => z= (lr+ls-lc+V(t1-t0))/2

     Passage mm->cm
  */

  if (type.compare("RE31") == 0)
  {
    memcpy(_clc, clc31, 48 * sizeof(double));
    memcpy(_cls, cls31, 48 * sizeof(double));
    memcpy(_clr, clr31, 48 * sizeof(double));
  }
  if (type.compare("RE41") == 0)
  {
    memcpy(_clc, clc41, 48 * sizeof(double));
    memcpy(_cls, cls41, 48 * sizeof(double));
    memcpy(_clr, clr41, 48 * sizeof(double));
  }

  double XT031[48] = {0.75, 12.135525, 23.521051, 34.906576, 46.292101, 57.677626, 69.063152, 80.448677, 91.834202, 103.219728, 114.605253, 125.990778, 137.376303, 148.761829, 160.147354, 171.532879, 182.918405, 194.30393, 205.689455, 217.07498, 228.460506, 239.846031, 251.231556, 262.617082, 274.002607, 285.388132, 296.773657, 308.159183, 319.544708, 330.930233, 342.315758, 353.701284, 365.086809, 376.472334, 387.85786, 399.243385, 410.62891, 422.014435, 433.399961, 444.785486, 456.171011, 467.556537, 478.942062, 490.327587, 501.713112, 511.520357, 520.962511, 530.335895};
  double YT031[48] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9.54039499999999, 20.82492, 32.027257};

  double XB031[48] = {0.75, 6.74246299999999, 12.734926, 18.727389, 24.719852, 30.712315, 36.704778, 42.697241, 48.689704, 54.682167, 60.67463, 66.667093, 72.659557, 78.65202, 84.644483, 90.636946, 96.629409, 102.621872, 108.614335, 114.606798, 120.599261, 126.591724, 132.584187, 138.57665, 144.569113, 150.561576, 156.554039, 162.546502, 168.538965, 174.531428, 180.523891, 186.516354, 192.508817, 198.50128, 204.493744, 210.486207, 216.47867, 222.471133, 228.463596, 234.456059, 241.025566, 247.902391, 254.827969, 261.802821, 268.827475, 275.902467, 283.028338, 290.20564};
  double YB031[48] = {1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1463.075864, 1457.304243, 1451.491705, 1445.637811, 1439.742119, 1433.80418, 1427.823538, 1421.799731};

  double XB131[48] = {6.24246299999999, 12.234926, 18.227389, 24.219852, 30.212315, 36.204778, 42.197241, 48.189704, 54.182167, 60.17463, 66.167093, 72.159557, 78.15202, 84.144483, 90.136946, 96.129409, 102.121872, 108.114335, 114.106798, 120.099261, 126.091724, 132.084187, 138.07665, 144.069113, 150.061576, 156.054039, 162.046502, 168.038965, 174.031428, 180.023891, 186.016354, 192.008817, 198.00128, 203.993744, 209.986207, 215.97867, 221.971133, 227.963596, 233.956059, 236.35, 247.329979, 254.253528, 261.226337, 268.248933, 275.321852, 282.445635, 289.620835, 296.848009};

  double YB131[48] = {1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1467, 1457.78466, 1451.973824, 1446.121646, 1440.227681, 1434.291482, 1428.312592, 1422.29055, 1416.224886};

  double XT131[48] = {11.635525, 23.021051, 34.406576, 45.792101, 57.177626, 68.563152, 79.948677, 91.334202, 102.719728, 114.105253, 125.490778, 136.876303, 148.261829, 159.647354, 171.032879, 182.418405, 193.80393, 205.189455, 216.57498, 227.960506, 239.346031, 250.731556, 262.117082, 273.502607, 284.888132, 296.273657, 307.659183, 319.044708, 330.430233, 341.815758, 353.201284, 364.586809, 375.972334, 387.35786, 398.743385, 410.12891, 421.514435, 432.899961, 444.285486, 455.671011, 467.056537, 478.442062, 489.827587, 501.213112, 503.537578, 520.546573, 529.921472, 539.22834};
  double YT131[48] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20.327824, 31.531972, 42.654814};

  double XT041[48] = {0.00, 12.00, 23.00, 34.00, 46.00, 57.00, 69.00, 80.00, 91.00, 103.00, 114.00, 125.00, 137.00, 148.00, 160.00, 171.00, 182.00, 194.00, 205.00, 217.00, 228.00, 239.00, 251.00, 262.00, 274.00, 285.00, 296.00, 308.00, 319.00, 330.00, 342.00, 353.00, 365.00, 376.00, 387.00, 399.00, 410.00, 422.00, 433.00, 444.00, 456.00, 467.00, 478.00, 490.00, 501.00, 511.00, 520.00, 530.00};
  double YT041[48] = {0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 10.00, 21.00, 32.00};
  double XB041[48] = {0.00, 7.00, 14.00, 21.00, 28.00, 35.00, 42.00, 49.00, 55.00, 62.00, 69.00, 76.00, 83.00, 90.00, 97.00, 104.00, 111.00, 118.00, 125.00, 131.00, 138.00, 145.00, 152.00, 159.00, 166.00, 173.00, 180.00, 187.00, 194.00, 200.00, 207.00, 214.00, 221.00, 228.00, 235.00, 242.00, 249.00, 256.00, 263.00, 270.00, 276.00, 284.00, 292.00, 300.00, 308.00, 316.00, 324.00, 333.00};
  double YB041[48] = {1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1216.00, 1209.00, 1202.00, 1195.00, 1189.00, 1182.00, 1175.00};
  double XT141[48] = {11.00, 23.00, 34.00, 45.00, 57.00, 68.00, 79.00, 91.00, 102.00, 114.00, 125.00, 136.00, 148.00, 159.00, 171.00, 182.00, 193.00, 205.00, 216.00, 227.00, 239.00, 250.00, 262.00, 273.00, 284.00, 296.00, 307.00, 319.00, 330.00, 341.00, 353.00, 364.00, 375.00, 387.00, 398.00, 410.00, 421.00, 432.00, 444.00, 455.00, 467.00, 478.00, 489.00, 501.00, 503.00, 520.00, 529.00, 539.00};
  double YT141[48] = {0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 21.00, 32.00, 43.00};
  double XB141[48] = {7.00, 14.00, 20.00, 27.00, 34.00, 41.00, 48.00, 55.00, 62.00, 69.00, 76.00, 83.00, 90.00, 96.00, 103.00, 110.00, 117.00, 124.00, 131.00, 138.00, 145.00, 152.00, 159.00, 165.00, 172.00, 179.00, 186.00, 193.00, 200.00, 207.00, 214.00, 221.00, 228.00, 235.00, 241.00, 248.00, 255.00, 262.00, 269.00, 276.00, 279.00, 291.00, 299.00, 307.00, 316.00, 324.00, 332.00, 340.00};
  double YB141[48] = {1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1220.00, 1209.00, 1203.00, 1196.00, 1189.00, 1182.00, 1175.00, 1168.00};

  if (type.compare("RE31") == 0)
  {
    memcpy(_XT0, XT031, 48 * sizeof(double));
    memcpy(_YT0, YT031, 48 * sizeof(double));
    memcpy(_XB0, XB031, 48 * sizeof(double));
    memcpy(_YB0, YB031, 48 * sizeof(double));
    memcpy(_XT1, XT131, 48 * sizeof(double));
    memcpy(_YT1, YT131, 48 * sizeof(double));
    memcpy(_XB1, XB131, 48 * sizeof(double));
    memcpy(_YB1, YB131, 48 * sizeof(double));
  }
  if (type.compare("RE41") == 0)
  {
    memcpy(_XT0, XT041, 48 * sizeof(double));
    memcpy(_YT0, YT041, 48 * sizeof(double));
    memcpy(_XB0, XB041, 48 * sizeof(double));
    memcpy(_YB0, YB041, 48 * sizeof(double));
    memcpy(_XT1, XT141, 48 * sizeof(double));
    memcpy(_YT1, YT141, 48 * sizeof(double));
    memcpy(_XB1, XB141, 48 * sizeof(double));
    memcpy(_YB1, YB141, 48 * sizeof(double));
  }

  _vPCB = 15.58;

  for (int strip_nb = 0; strip_nb < 48; strip_nb++)
  {

    _xtop[strip_nb] = (_XT0[strip_nb] + _XT1[strip_nb]) / 20.;
    _ytop[strip_nb] = (_YT0[strip_nb] + _YT1[strip_nb]) / 20.;
    _xbottom[strip_nb] = (_XB0[strip_nb] + _XB1[strip_nb]) / 20.;
    _ybottom[strip_nb] = (_YB0[strip_nb] + _YB1[strip_nb]) / 20.;

    _lstrip[strip_nb] = sqrt((_xbottom[strip_nb] - _xtop[strip_nb]) * (_xbottom[strip_nb] - _xtop[strip_nb]) + (_ybottom[strip_nb] - _ytop[strip_nb]) * (_ybottom[strip_nb] - _ytop[strip_nb]));

    _cost[strip_nb] = (_xbottom[strip_nb] - _xtop[strip_nb]) / _lstrip[strip_nb];
    _sint[strip_nb] = (_ybottom[strip_nb] - _ytop[strip_nb]) / _lstrip[strip_nb];
  }
  if (side.compare("LEFT") == 0)
    _side = 0;
  else
    _side = 1;
}
void lmana::irpcGeometry::initialize(std::string name, Json::Value jv)
{
  // Calculate position inside the strip
  /*
     t1 is T High radius
     t0 is T Low radius

     lc: distance High radius of strip to connector
     ls : strip length
     lr : distance Low radius of strip to connector
     z : distance along the strip from High radius

     t1= (lc+z)/V
     t0= (ls-z+lr)/V

     => V(t1-t0)=lc+2z -ls -lr

     => z= (lr+ls-lc+V(t1-t0))/2

     Passage mm->cm
  */

   
  //LM_INFO(_logGeo,"debug");
  //std::cout<<jv["LC"]<<std::endl;
  for (int i = 0; i < 48; i++)
  {

    _clc[i] = jv["LC"][i].asDouble();


    _clr[i] = jv["LR"][i].asDouble();

    _cls[i] = jv["LS"][i].asDouble();

    _XT0[i] = jv["XT0"][i].asDouble();

    _YT0[i] = jv["YT0"][i].asDouble();

    _XB0[i] = jv["XB0"][i].asDouble();

    _YB0[i] = jv["YB0"][i].asDouble();

    _XT1[i] = jv["XT1"][i].asDouble();

    _YT1[i] = jv["YT1"][i].asDouble();

    _XB1[i] = jv["XB1"][i].asDouble();

    _YB1[i] = jv["YB1"][i].asDouble();

  }

  _vPCB = 15.58;

  for (int strip_nb = 0; strip_nb < 48; strip_nb++)
  {

    _xtop[strip_nb] = (_XT0[strip_nb] + _XT1[strip_nb]) / 20.;
    _ytop[strip_nb] = (_YT0[strip_nb] + _YT1[strip_nb]) / 20.;
    _xbottom[strip_nb] = (_XB0[strip_nb] + _XB1[strip_nb]) / 20.;
    _ybottom[strip_nb] = (_YB0[strip_nb] + _YB1[strip_nb]) / 20.;

    _lstrip[strip_nb] = sqrt((_xbottom[strip_nb] - _xtop[strip_nb]) * (_xbottom[strip_nb] - _xtop[strip_nb]) + (_ybottom[strip_nb] - _ytop[strip_nb]) * (_ybottom[strip_nb] - _ytop[strip_nb]));
    //printf("%d %f %f %f %f %f \n",strip_nb,_xtop[strip_nb],_ytop[strip_nb],_xbottom[strip_nb],_ybottom[strip_nb],_lstrip[strip_nb]);
    _cost[strip_nb] = (_xbottom[strip_nb] - _xtop[strip_nb]) / _lstrip[strip_nb];
    _sint[strip_nb] = (_ybottom[strip_nb] - _ytop[strip_nb]) / _lstrip[strip_nb];
  }
  //getchar();
  if ((name.compare("RE31_LEFT") == 0) || (name.compare("RE41_LEFT") == 0))
    _side = 0;
  else
    _side = 1;
  std::cout<<"#Parsing of "<<name<<std::endl;
}
#undef ONESPEED
void lmana::irpcGeometry::localPosition(uint32_t strip, double t0, double t1, double &zs, double &xloc, double &yloc,double dzs)
{
  //fprintf(stderr,"%d %f %f Side %d \n",strip,t0,t1,_side);
  //getchar();
  // Strip in array of XTOF
  uint32_t strip_nb = strip; // 47-_str;
  if (_side == 0)
    strip_nb = 47 - strip;

  // Calculate position inside the strip
  /*
     t1 is T High radius
     t0 is T Low radius

     lc: distance High radius of strip to connector
     ls : strip length
     lr : distance Low radius of strip to connector
     z : distance along the strip from High radius

     t1= (lc+z)/V
     t0= (ls-z+lr)/V

     => V(t1-t0)=lc+2z -ls -lr

     => z= (lr+ls-lc+V(t1-t0))/2

     Passage mm->cm
  */

  double Lr = _clr[strip_nb] / 10.;
  double Ls = _cls[strip_nb] / 10.;
  double Lc = _clc[strip_nb] / 10.;
#ifdef ONESPEED
  zs = (Ls + Lr - Lc + _vPCB * (t1 - t0)) / 2.0;
#else
  double vp=17.148,vr=14.090;
  zs=(Ls-vp/vr*(Lc-Lr)+vp*(t1-t0))/2.;
#endif
  zs=zs-dzs;
  // Strips poligon
  /*
PT1     PT0
+++++
 \   \
  \   \
   \   \
   +++++
 PB1     PB0

      WE calculate:

       - P Top ( (xt1+xt0)/2,(yt1+yt0)/2)
       - P Bot  ( (xb1+xb0)/2,(yb1+yb0)/2)

       Then Cos(theta) = (Xbot-Xtop)/Lstrip  Sin(theta)= (Ybot-Ytop)/Lstrip

      and
        X = Xtop+cos(theta)*z
  Y = Ytop+sin(theta)*z
    */
  // Essai
  xloc = (_xtop[strip_nb]) + zs * _cost[strip_nb];
  yloc = _ytop[strip_nb] + zs * _sint[strip_nb];
}
void lmana::irpcGeometry::delays_extrema(double dti[], double dta[])
{
  uint32_t strip_nb = 0;
  for (int is = 0; is < 48; is++)
  {
    if (_side == 0)
      strip_nb = 47 - is;

    double zmin = 0;
    double zmax = (_xbottom[strip_nb] - _xtop[strip_nb]) / _cost[strip_nb];

    double Lr = _clr[strip_nb] / 10.;
    double Ls = _cls[strip_nb] / 10.;
    double Lc = _clc[strip_nb] / 10.;

    double vp = 17.148, vr = 14.090;

    double dtmax = -1. * (2 * zmax - (Ls - vp / vr * (Lc - Lr))) / vp;
    double dtmin = -1. * (2 * zmin - (Ls - vp / vr * (Lc - Lr))) / vp;
    //printf("Strip %d  ZM %.2f %.2f DT %.2f %.2f \n", is, zmin, zmax, dtmin, dtmax);
    dti[is] = dtmax-1.;
    dta[is] = dtmin+1.;
  }
}
void lmana::irpcGeometry::dump_extremas()
{

  double dti[48], dta[48];
  double ymi=0,yma=1520;
  for (int is = 0; is < 48; is++)
  {
    uint32_t strip_nb=is;
    if (_side == 0)
      strip_nb = 47 - is;

    double zmin = 0;
    double zmax = (_xbottom[strip_nb] - _xtop[strip_nb]) / _cost[strip_nb];
    //printf("%d %f %f  %f \n",strip_nb,_xbottom[strip_nb],_xtop[strip_nb], _cost[strip_nb]);
    double Lr = _clr[strip_nb] / 10.;
    double Ls = _cls[strip_nb] / 10.;
    double Lc = _clc[strip_nb] / 10.;

    double vp = 17.148, vr = 14.090;

    double dtmax = -1. * (2 * zmax - (Ls - vp / vr * (Lc - Lr))) / vp;
    double dtmin = -1. * (2 * zmin - (Ls - vp / vr * (Lc - Lr))) / vp;
    //printf("Strip %d YM %.2f %.2f ZM %.2f %.2f DT %.2f %.2f \n", is, ymi, yma, zmin, zmax, dtmin, dtmax);
    dti[is] = dtmax;
    dta[is] = dtmin;
  }
  printf("dt_min=[");
  for (int i = 0; i < 47; i++)
    printf("%.2f,", dti[i]);
  printf("%.2f];\n", dti[47]);
  printf("dt_max=[");
  for (int i = 0; i < 47; i++)
    printf("%.2f,", dta[i]);
  printf("%.2f];\n", dta[47]);
}
void lmana::irpcGeometry::maxima(uint32_t strip,double &ymi, double &yma)
{
  uint32_t strip_nb = strip; // 47-_str;
  if (_side == 0)
    strip_nb = 47 - strip;
  ymi=_ytop[strip_nb];
  yma=_ybottom[strip_nb];

#ifdef DUMPDELAYS
  double dti[48],dta[48];
  for (int is=0;is<48;is++)
    {
      if (_side == 0)
	strip_nb = 47 - is;
      
      double zmin=0;
      double zmax=  (_xbottom[strip_nb]-_xtop[strip_nb])/_cost[strip_nb];
      
      double Lr = _clr[strip_nb] / 10.;
      double Ls = _cls[strip_nb] / 10.;
      double Lc = _clc[strip_nb] / 10.;
      
      double vp=17.148,vr=14.090;
      
      double dtmax=-1.*(2*zmax-(Ls-vp/vr*(Lc-Lr)))/vp;
      double dtmin=-1.*(2*zmin-(Ls-vp/vr*(Lc-Lr)))/vp;
      printf("Strip %d YM %.2f %.2f ZM %.2f %.2f DT %.2f %.2f \n",is,ymi,yma,zmin,zmax,dtmin,dtmax);
      dti[is]=dtmax;
      dta[is]=dtmin;
    }
  printf("dt_min=[");
  for (int i=0;i<47;i++)
    printf("%.2f,",dti[i]);
  printf("%.2f];\n",dti[47]);
  printf("dt_max=[");
  for (int i=0;i<47;i++)
    printf("%.2f,",dta[i]);
  printf("%.2f];\n",dta[47]);
  getchar();
#endif
}
void lmana::irpcGeometry::fromLocalPosition(double xloc, double yloc,uint32_t &strip,double &zs)
{
  strip = 99;
  double bestDist2 = 1e12;
  int bestStrip = -1;
  double bestZ = 0.0;

  auto cross = [&](double ax, double ay, double bx, double by) {
    return ax * by - ay * bx;
  };

  for (uint32_t is = 0; is < 48; ++is)
  {
    uint32_t strip_nb = is;
    if (_side == 0)
      strip_nb = 47 - is;

    double x0 = _XT0[strip_nb] / 10.;
    double y0 = _YT0[strip_nb] / 10.;
    double x1 = _XT1[strip_nb] / 10.;
    double y1 = _YT1[strip_nb] / 10.;
    double x2 = _XB1[strip_nb] / 10.;
    double y2 = _YB1[strip_nb] / 10.;
    double x3 = _XB0[strip_nb] / 10.;
    double y3 = _YB0[strip_nb] / 10.;

    double c0 = cross(x1 - x0, y1 - y0, xloc - x0, yloc - y0);
    double c1 = cross(x2 - x1, y2 - y1, xloc - x1, yloc - y1);
    double c2 = cross(x3 - x2, y3 - y2, xloc - x2, yloc - y2);
    double c3 = cross(x0 - x3, y0 - y3, xloc - x3, yloc - y3);

    double zproj = (xloc - _xtop[strip_nb]) * _cost[strip_nb] + (yloc - _ytop[strip_nb]) * _sint[strip_nb];
    if (zproj < 0.)
      zproj = 0.;
    if (zproj > _lstrip[strip_nb])
      zproj = _lstrip[strip_nb];

    bool inside = ((c0 >= 0 && c1 >= 0 && c2 >= 0 && c3 >= 0) || (c0 <= 0 && c1 <= 0 && c2 <= 0 && c3 <= 0));
    if (inside)
    {
      bestStrip = is;
      bestZ = zproj;
      bestDist2 = 0.0;
      break;
    }

    double px = _xtop[strip_nb] + zproj * _cost[strip_nb];
    double py = _ytop[strip_nb] + zproj * _sint[strip_nb];
    double dist2 = (xloc - px) * (xloc - px) + (yloc - py) * (yloc - py);
    if (dist2 < bestDist2)
    {
      bestDist2 = dist2;
      bestStrip = is;
      bestZ = zproj;
    }
  }

  if (bestStrip < 0)
  {
    strip = 23;
    zs = 0.;
    return;
  }

  strip = bestStrip;
  zs = bestZ;
  if (_side == 0)
    strip = 47 - strip;
}

void lmana::irpcGeometry::fromLocPos1(double xloc, double yloc,uint32_t &strip,double &zs)
{
  strip = 99;
  double bestDist2 = 1e12;
  int bestStrip = -1;
  double bestZ = 0.0;

  auto cross = [&](double ax, double ay, double bx, double by) {
    return ax * by - ay * bx;
  };

  for (uint32_t is = 0; is < 48; ++is)
  {
    uint32_t strip_nb = is;
    if (_side == 0)
      strip_nb = 47 - is;

    double x0 = _XT0[strip_nb] / 10.;
    double y0 = _YT0[strip_nb] / 10.;
    double x1 = _XT1[strip_nb] / 10.;
    double y1 = _YT1[strip_nb] / 10.;
    double x2 = _XB1[strip_nb] / 10.;
    double y2 = _YB1[strip_nb] / 10.;
    double x3 = _XB0[strip_nb] / 10.;
    double y3 = _YB0[strip_nb] / 10.;

    double c0 = cross(x1 - x0, y1 - y0, xloc - x0, yloc - y0);
    double c1 = cross(x2 - x1, y2 - y1, xloc - x1, yloc - y1);
    double c2 = cross(x3 - x2, y3 - y2, xloc - x2, yloc - y2);
    double c3 = cross(x0 - x3, y0 - y3, xloc - x3, yloc - y3);

    double zproj = (xloc - _xtop[strip_nb]) * _cost[strip_nb] + (yloc - _ytop[strip_nb]) * _sint[strip_nb];
    if (zproj < 0.)
      zproj = 0.;
    if (zproj > _lstrip[strip_nb])
      zproj = _lstrip[strip_nb];

    bool inside = ((c0 >= 0 && c1 >= 0 && c2 >= 0 && c3 >= 0) ||
                   (c0 <= 0 && c1 <= 0 && c2 <= 0 && c3 <= 0));
    if (inside)
    {
      bestStrip = is;
      bestZ = zproj;
      bestDist2 = 0.0;
      break;
    }

    double px = _xtop[strip_nb] + zproj * _cost[strip_nb];
    double py = _ytop[strip_nb] + zproj * _sint[strip_nb];
    double dist2 = (xloc - px) * (xloc - px) + (yloc - py) * (yloc - py);
    if (dist2 < bestDist2)
    {
      bestDist2 = dist2;
      bestStrip = is;
      bestZ = zproj;
    }
  }

  if (bestStrip < 0)
  {
    strip = 23;
    zs = 0.;
    return;
  }

  strip = bestStrip;
  zs = bestZ;
  if (_side == 0)
    strip = 47 - strip;
}
