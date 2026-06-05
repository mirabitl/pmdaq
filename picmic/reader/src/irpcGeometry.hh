#pragma once
#include <json/json.h>
#include <stdint.h>
#include <string>
#include <Math/PositionVector3D.h>
#include <Math/Point3Dfwd.h>
#include <Math/Vector3Dfwd.h>
#include <Math/DisplacementVector3D.h>
#include "febv2Macro.hh"
//#include "lmLogger.hh"
#include <memory>



namespace lmana
{
  /*!
    \class  irpcGeometry
    \author  L.Mirabito
    \date October 2021
    \version 1.0

    \brief Access of the geometry of an iRPC
    <h2> Description</h2>
    It gives all tools to build hit position from strip number t0(High Radius) and t1(Low Radius) values
    The geometry is hard coded but can also be read from a JSON file
  */
  class irpcGeometry
  {
  public:
    /// @brief Default Constructor
    irpcGeometry();

    /// @brief Initialise all strip length and angles from hard-coded array
    /// @param type RE31 or RE41
    /// @param side LEFT or RIGHT
    void initialize(std::string type, std::string side);

    /// @brief Initialise all strip length and angles from JSON value
    /// @param name RE31_LEFT .... RE41_RIGHT
    /// @param jv JSON parsed value of the irpc Geometry
    void initialize(std::string name, Json::Value jv);

    /// @brief Retrieve chamber hit position using the intialised values
    /// @param strip strip number
    /// @param t0 t HR
    /// @param t1 t LR
    /// @param zs return z of the plane
    /// @param xloc calculated x position
    /// @param yloc calculated y position
    void localPosition(uint32_t strip, double t0, double t1, double &zs, double &xloc, double &yloc,double dzs=0);
    void fromLocalPosition(double xloc, double yloc,uint32_t &strip,double &zs);
    void fromLocPos1(double xloc, double yloc,uint32_t &strip,double &zs);
    void maxima(uint32_t strip,double &ymi, double &yma);
    void delays_extrema(double dti[], double dta[]);
    void dump_extremas();

  private:
    double _clc[48], _cls[48], _clr[48];
    double _XT0[48], _XB0[48], _YT0[48], _YB0[48];
    double _XT1[48], _XB1[48], _YT1[48], _YB1[48];
    double _xtop[48], _ytop[48], _xbottom[48], _ybottom[48], _lstrip[48], _cost[48], _sint[48];
    uint32_t _side;
    double _vPCB;
  };
};
  
