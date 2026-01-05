import math
import json

class IrpcGeometry:
    def __init__(self):
        # Initialisation des tableaux
        self._clc = [0.0] * 48
        self._clr = [0.0] * 48
        self._cls = [0.0] * 48
        self._XT0 = [0.0] * 48
        self._YT0 = [0.0] * 48
        self._XB0 = [0.0] * 48
        self._YB0 = [0.0] * 48
        self._XT1 = [0.0] * 48
        self._YT1 = [0.0] * 48
        self._XB1 = [0.0] * 48
        self._YB1 = [0.0] * 48
        self._xtop = [0.0] * 48
        self._ytop = [0.0] * 48
        self._xbottom = [0.0] * 48
        self._ybottom = [0.0] * 48
        self._lstrip = [0.0] * 48
        self._cost = [0.0] * 48
        self._sint = [0.0] * 48
        self._vPCB = 0.0
        self._side = 0

    def initialize(self, type_, side):
        # Données statiques (exemple pour RE31 et RE41)
        clc31 = [90.543564, 80.410461, 70.289078, 60.173553, 50.069733, 40.903408, 36.713425, 32.523438, 31.166557, 36.356541, 40.546543, 49.202293, 59.317810, 69.433334, 79.548859, 89.664398, 56.088192, 66.203720, 76.319244, 86.428909, 96.550293, 106.665825, 116.775482, 126.896873, 137.006546, 147.122055, 157.243454, 167.358978, 177.474487, 187.590027, 197.699677, 207.821075, 175.118668, 185.379196, 195.481796, 205.597305, 215.848694, 225.834229, 235.949753, 246.065292, 256.180817, 266.302185, 276.671875, 286.533234, 301.558472, 314.650421, 328.493317, 343.113739]
        clc41 = [90.894295, 80.668900, 70.553459, 60.438061, 50.316757, 40.429695, 36.239754, 0.890015, 31.640133, 34.830097, 39.020042, 44.321323, 57.034809, 68.150246, 77.265640, 79.041321, 56.276508, 64.931931, 75.047348, 85.162766, 95.278206, 105.393593, 115.509033, 125.624458, 135.739899, 145.855316, 155.970718, 166.086151, 143.920959, 187.317017, 141.053879, 207.547836, 0.880005, 184.098709, 194.214096, 204.329529, 214.444931, 224.560394, 234.675842, 244.791199, 10.737075, 265.022095, 275.137512, 285.252930, 300.251160, 313.273132, 327.085388, 340.790131]
        clr31 = [2313.407715, 2303.935059, 2294.468262, 2285.000977, 2275.528320, 2266.067383, 2256.606445, 2247.133789, 2238.666504, 2228.200195, 2218.733398, 2209.266113, 2199.787598, 2190.332520, 2180.877441, 2171.387207, 2120.251953, 2110.785156, 2101.318359, 2091.851074, 2082.384277, 2072.917480, 2063.444824, 2053.983887, 2044.516846, 2035.049927, 2025.583008, 2016.116333, 2006.649414, 1997.182495, 1987.721558, 1978.243042, 1926.101929, 1916.629150, 1907.162231, 1897.701416, 1888.234497, 1878.773438, 1869.300659, 1855.762939, 1844.198853, 1832.120117, 1819.977539, 1807.770386, 1795.510132, 1783.159668, 1770.754272, 1757.014038]
        cls31 = [1468.002441, 1468.022339, 1468.062012, 1468.121460, 1468.200928, 1468.300049, 1468.419067, 1468.557861, 1467.717041, 1468.895020, 1469.093262, 1469.311279, 1469.549194, 1469.806763, 1470.084229, 1470.381348, 1470.698242, 1471.034790, 1471.391235, 1471.767212, 1472.162964, 1472.578247, 1473.013306, 1473.468018, 1473.942383, 1474.436279, 1474.949707, 1475.482788, 1476.035400, 1476.607544, 1477.199097, 1477.810181, 1477.132324, 1479.090576, 1479.760010, 1480.448730, 1480.165649, 1481.884155, 1482.630737, 1481.655518, 1477.538940, 1472.481323, 1466.406738, 1462.278442, 1452.549316, 1436.809814, 1420.182617, 1403.590942]
        XT031 = [0.75, 12.135525, 23.521051, 34.906576, 46.292101, 57.677626, 69.063152, 80.448677, 91.834202, 103.219728, 114.605253, 125.990778, 137.376303, 148.761829, 160.147354, 171.532879, 182.918405, 194.30393, 205.689455, 217.07498, 228.460506, 239.846031, 251.231556, 262.617082, 274.002607, 285.388132, 296.773657, 308.159183, 319.544708, 330.930233, 342.315758, 353.701284, 365.086809, 376.472334, 387.85786, 399.243385, 410.62891, 422.014435, 433.399961, 444.785486, 456.171011, 467.556537, 478.942062, 490.327587, 501.713112, 511.520357, 520.962511, 530.335895]
        YT031 = [0] * 48
        XB031 = [0.75, 6.74246299999999, 12.734926, 18.727389, 24.719852, 30.712315, 36.704778, 42.697241, 48.689704, 54.682167, 60.67463, 66.667093, 72.659557, 78.65202, 84.644483, 90.636946, 96.629409, 102.621872, 108.614335, 114.606798, 120.599261, 126.591724, 132.584187, 138.57665, 144.569113, 150.561576, 156.554039, 162.546502, 168.538965, 174.531428, 180.523891, 186.516354, 192.508817, 198.50128, 204.493744, 210.486207, 216.47867, 222.471133, 228.463596, 234.456059, 241.025566, 247.902391, 254.827969, 261.802821, 268.827475, 275.902467, 283.028338, 290.20564]
        YB031 = [1467] * 48
        XT131 = [11.635525, 23.021051, 34.406576, 45.792101, 57.177626, 68.563152, 79.948677, 91.334202, 102.719728, 114.105253, 125.490778, 136.876303, 148.261829, 159.647354, 171.032879, 182.418405, 193.80393, 205.189455, 216.57498, 227.960506, 239.346031, 250.731556, 262.117082, 273.502607, 284.888132, 296.273657, 307.659183, 319.044708, 330.430233, 341.815758, 353.201284, 364.586809, 375.972334, 387.35786, 398.743385, 410.12891, 421.514435, 432.899961, 444.285486, 455.671011, 467.056537, 478.442062, 489.827587, 501.213112, 503.537578, 520.546573, 529.921472, 539.22834]
        YT131 = [0] * 48
        XB131 = [6.24246299999999, 12.234926, 18.227389, 24.219852, 30.212315, 36.204778, 42.197241, 48.189704, 54.182167, 60.17463, 66.167093, 72.159557, 78.15202, 84.144483, 90.136946, 96.129409, 102.121872, 108.114335, 114.106798, 120.099261, 126.091724, 132.084187, 138.07665, 144.069113, 150.061576, 156.054039, 162.046502, 168.038965, 174.031428, 180.023891, 186.016354, 192.008817, 198.00128, 203.993744, 209.986207, 215.97867, 221.971133, 227.963596, 233.956059, 236.35, 247.329979, 254.253528, 261.226337, 268.248933, 275.321852, 282.445635, 289.620835, 296.848009]
        YB131 = [1467] * 48

        if type_ == "RE31":
            self._clc = clc31.copy()
            self._cls = cls31.copy()
            self._clr = clr31.copy()
            self._XT0 = XT031.copy()
            self._YT0 = YT031.copy()
            self._XB0 = XB031.copy()
            self._YB0 = YB031.copy()
            self._XT1 = XT131.copy()
            self._YT1 = YT131.copy()
            self._XB1 = XB131.copy()
            self._YB1 = YB131.copy()
        elif type_ == "RE41":
            # Ajoutez ici les données pour RE41 si nécessaire
            pass

        self._vPCB = 15.58

        for strip_nb in range(48):
            self._xtop[strip_nb] = (self._XT0[strip_nb] + self._XT1[strip_nb]) / 20.
            self._ytop[strip_nb] = (self._YT0[strip_nb] + self._YT1[strip_nb]) / 20.
            self._xbottom[strip_nb] = (self._XB0[strip_nb] + self._XB1[strip_nb]) / 20.
            self._ybottom[strip_nb] = (self._YB0[strip_nb] + self._YB1[strip_nb]) / 20.

            self._lstrip[strip_nb] = math.sqrt((self._xbottom[strip_nb] - self._xtop[strip_nb]) ** 2 + (self._ybottom[strip_nb] - self._ytop[strip_nb]) ** 2)

            self._cost[strip_nb] = (self._xbottom[strip_nb] - self._xtop[strip_nb]) / self._lstrip[strip_nb]
            self._sint[strip_nb] = (self._ybottom[strip_nb] - self._ytop[strip_nb]) / self._lstrip[strip_nb]

        if side == "LEFT":
            self._side = 0
        else:
            self._side = 1

    def initialize_from_json(self, name, jv):
        for i in range(48):
            self._clc[i] = jv["LC"][i]
            self._clr[i] = jv["LR"][i]
            self._cls[i] = jv["LS"][i]
            self._XT0[i] = jv["XT0"][i]
            self._YT0[i] = jv["YT0"][i]
            self._XB0[i] = jv["XB0"][i]
            self._YB0[i] = jv["YB0"][i]
            self._XT1[i] = jv["XT1"][i]
            self._YT1[i] = jv["YT1"][i]
            self._XB1[i] = jv["XB1"][i]
            self._YB1[i] = jv["YB1"][i]

        self._vPCB = 15.58

        for strip_nb in range(48):
            self._xtop[strip_nb] = (self._XT0[strip_nb] + self._XT1[strip_nb]) / 20.
            self._ytop[strip_nb] = (self._YT0[strip_nb] + self._YT1[strip_nb]) / 20.
            self._xbottom[strip_nb] = (self._XB0[strip_nb] + self._XB1[strip_nb]) / 20.
            self._ybottom[strip_nb] = (self._YB0[strip_nb] + self._YB1[strip_nb]) / 20.

            self._lstrip[strip_nb] = math.sqrt((self._xbottom[strip_nb] - self._xtop[strip_nb]) ** 2 + (self._ybottom[strip_nb] - self._ytop[strip_nb]) ** 2)

            self._cost[strip_nb] = (self._xbottom[strip_nb] - self._xtop[strip_nb]) / self._lstrip[strip_nb]
            self._sint[strip_nb] = (self._ybottom[strip_nb] - self._ytop[strip_nb]) / self._lstrip[strip_nb]

        if name in ["RE31_LEFT", "RE41_LEFT"]:
            self._side = 0
        else:
            self._side = 1

    def local_position(self, strip, t0, t1, dzs=0.0):
        strip_nb = strip
        if self._side == 0:
            strip_nb = 47 - strip

        Lr = self._clr[strip_nb] / 10.
        Ls = self._cls[strip_nb] / 10.
        Lc = self._clc[strip_nb] / 10.
        vp, vr = 17.148, 14.090
        zs = (Ls - vp / vr * (Lc - Lr) + vp * (t1 - t0)) / 2.
        zs -= dzs

        xloc = self._xtop[strip_nb] + zs * self._cost[strip_nb]
        yloc = self._ytop[strip_nb] + zs * self._sint[strip_nb]

        return zs, xloc, yloc

    def delays_extrema(self):
        dti = [0.0] * 48
        dta = [0.0] * 48
        for is_ in range(48):
            strip_nb = is_
            if self._side == 0:
                strip_nb = 47 - is_

            zmin = 0
            zmax = (self._xbottom[strip_nb] - self._xtop[strip_nb]) / self._cost[strip_nb]

            Lr = self._clr[strip_nb] / 10.
            Ls = self._cls[strip_nb] / 10.
            Lc = self._clc[strip_nb] / 10.
            vp, vr = 17.148, 14.090

            dtmax = -1. * (2 * zmax - (Ls - vp / vr * (Lc - Lr))) / vp
            dtmin = -1. * (2 * zmin - (Ls - vp / vr * (Lc - Lr))) / vp

            dti[is_] = dtmax - 1.
            dta[is_] = dtmin + 1.

        return dti, dta

    def dump_extremas(self):
        dti, dta = self.delays_extrema()
        print("dt_min=[", end="")
        for i in range(47):
            print(f"{dti[i]:.2f},", end="")
        print(f"{dti[47]:.2f}];")

        print("dt_max=[", end="")
        for i in range(47):
            print(f"{dta[i]:.2f},", end="")
        print(f"{dta[47]:.2f}];")

    def maxima(self, strip):
        strip_nb = strip
        if self._side == 0:
            strip_nb = 47 - strip

        ymi = self._ytop[strip_nb]
        yma = self._ybottom[strip_nb]

        return ymi, yma

    def from_local_position(self, xloc, yloc):
        strip = 99
        found = False
        vs = []
        for is_ in range(48):
            if xloc > (self._XB0[is_] / 10. - 0.1) and xloc < (self._XT1[is_] / 10. + 0.1):
                vs.append(is_)

        for is_ in vs:
            if is_ < 47:
                xt1 = self._XT0[is_ + 1]
                yt1 = self._YT0[is_ + 1]
                xb1 = self._XB0[is_ + 1]
                yb1 = self._YB0[is_ + 1]
            else:
                xt1 = self._XT1[is_]
                yt1 = self._YT1[is_]
                xb1 = self._XB1[is_]
                yb1 = self._YB1[is_]

            v1x = (self._XB0[is_] - self._XT0[is_]) / 10.
            v1y = (self._YB0[is_] - self._YT0[is_]) / 10.
            vpx = xloc - self._XT0[is_] / 10.
            vpy = yloc - self._YT0[is_] / 10.
            d1 = v1x * vpy - v1y * vpx

            v2x = (xb1 - self._XB0[is_]) / 10.
            v2y = (yb1 - self._YB0[is_]) / 10.
            vpx = xloc - self._XB0[is_] / 10.
            vpy = yloc - self._YB0[is_] / 10.
            d2 = v2x * vpy - v2y * vpx

            v3x = (xt1 - xb1) / 10.
            v3y = (yt1 - yb1) / 10.
            vpx = xloc - xb1 / 10.
            vpy = yloc - yb1 / 10.
            d3 = v3x * vpy - v3y * vpx

            v4x = (self._XT0[is_] - xt1) / 10.
            v4y = (self._YT0[is_] - yt1) / 10.
            vpx = xloc - xt1 / 10.
            vpy = yloc - yt1 / 10.
            d4 = v4x * vpy - v4y * vpx

            if d1 * d2 * d3 * d4 > 0:
                strip = is_
                found = True
                break

        if not found:
            strip = 23

        zs = (yloc - self._ytop[strip]) / self._sint[strip]
        if self._side == 0:
            strip = 47 - strip

        return strip, zs
