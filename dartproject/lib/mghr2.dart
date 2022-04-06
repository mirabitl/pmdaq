import 'package:mgaccess/mgasic.dart' as mga;
import 'dart:convert';
import 'package:mongo_dart/mongo_dart.dart'
    show Db, WriteConcern, modify, where;
import 'package:ansicolor/ansicolor.dart';
import 'dart:io';
import 'package:logging/logging.dart';
import 'package:json_sorter/json_sorter.dart';

class mgHR2 extends mga.mgRoc {
  final difList = Map<int, dynamic>();
  final asicIds = List.empty(growable: true);
  late String currentStateName;
  late int currentVersion;

  /// Constructor
  mgHR2(String account) : super(account) {}
  Map<String, dynamic> buildHR2(int num, {int gain = 128}) {
    var _jasic = new Map<String, dynamic>();
    _jasic["ENABLED"] = 1;
    _jasic["HEADER"] = num;
    _jasic["QSCSROUTSC"] = 1;
    _jasic["ENOCDOUT1B"] = 1;
    _jasic["ENOCDOUT2B"] = 0;
    _jasic["ENOCTRANSMITON1B"] = 1;
    _jasic["ENOCTRANSMITON2B"] = 0;
    _jasic["ENOCCHIPSATB"] = 1;
    _jasic["SELENDREADOUT"] = 1;
    _jasic["SELSTARTREADOUT"] = 1;
    _jasic["CLKMUX"] = 1;
    _jasic["SCON"] = 0;
    _jasic["RAZCHNEXTVAL"] = 0;
    _jasic["RAZCHNINTVAL"] = 1;
    _jasic["TRIGEXTVAL"] = 0;
    _jasic["DISCROROR"] = 1;
    _jasic["ENTRIGOUT"] = 1;
    _jasic["TRIG0B"] = 1;
    _jasic["TRIG1B"] = 0;
    _jasic["TRIG2B"] = 0;
    _jasic["OTABGSW"] = 0;
    _jasic["DACSW"] = 0;
    _jasic["SMALLDAC"] = 0;
    _jasic["B2"] = 250;
    _jasic["B1"] = 250;
    _jasic["B0"] = 250;
    var mask = List.filled(64, 1);

    _jasic["MASK0"] = mask;
    _jasic["MASK1"] = mask;
    _jasic["MASK2"] = mask;
    _jasic["RS_OR_DISCRI"] = 1;
    _jasic["DISCRI1"] = 0;
    _jasic["DISCRI2"] = 0;
    _jasic["DISCRI0"] = 0;
    _jasic["OTAQ_PWRADC"] = 0;
    _jasic["EN_OTAQ"] = 1;
    _jasic["SW50F0"] = 1;
    _jasic["SW100F0"] = 1;
    _jasic["SW100K0"] = 1;
    _jasic["SW50K0"] = 1;
    _jasic["PWRONFSB1"] = 0;
    _jasic["PWRONFSB2"] = 0;
    _jasic["PWRONFSB0"] = 0;
    _jasic["SEL1"] = 0;
    _jasic["SEL0"] = 1;
    _jasic["SW50F1"] = 1;
    _jasic["SW100F1"] = 1;
    _jasic["SW100K1"] = 1;
    _jasic["SW50K1"] = 1;
    _jasic["CMDB0FSB1"] = 1;
    _jasic["CMDB1FSB1"] = 1;
    _jasic["CMDB2FSB1"] = 0;
    _jasic["CMDB3FSB1"] = 1;
    _jasic["SW50F2"] = 1;
    _jasic["SW100F2"] = 1;
    _jasic["SW100K2"] = 1;
    _jasic["SW50K2"] = 1;
    _jasic["CMDB0FSB2"] = 1;
    _jasic["CMDB1FSB2"] = 1;
    _jasic["CMDB2FSB2"] = 0;
    _jasic["CMDB3FSB2"] = 1;
    _jasic["PWRONW"] = 0;
    _jasic["PWRONSS"] = 0;
    _jasic["PWRONBUFF"] = 0;
    _jasic["SWSSC"] = 7;
    _jasic["CMDB0SS"] = 0;
    _jasic["CMDB1SS"] = 0;
    _jasic["CMDB2SS"] = 0;
    _jasic["CMDB3SS"] = 0;
    _jasic["PWRONPA"] = 0;
    //Unset power pulsing
    _jasic["CLKMUX"] = 1;
    _jasic["SCON"] = 1;
    _jasic["OTAQ_PWRADC"] = 1;
    _jasic["PWRONW"] = 1;
    _jasic["PWRONSS"] = 0;
    _jasic["PWRONBUFF"] = 1;
    _jasic["PWRONPA"] = 1;
    _jasic["DISCRI0"] = 1;
    _jasic["DISCRI1"] = 1;
    _jasic["DISCRI2"] = 1;
    _jasic["OTABGSW"] = 1;
    _jasic["DACSW"] = 1;
    _jasic["PWRONFSB0"] = 1;
    _jasic["PWRONFSB1"] = 1;
    _jasic["PWRONFSB2"] = 1;

    var dv = List<int>.filled(32, gain);
    var ct = List<int>.filled(64, 0);

    _jasic["PAGAIN"] = dv;
    _jasic["CTEST"] = ct;

    return _jasic;
  }
}
