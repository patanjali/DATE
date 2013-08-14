//////////////////////////////////////////////////////////////////
//
//
//  Helper functions and classes to parse the ISPD 2013 contest
//  benchmark files.
//
//  This code is provided for description purposes only. The contest
//  organizers cannot guarantee that the provided code is free of
//  bugs or defects. !!!! USE THIS CODE AT YOUR OWN RISK !!!!!
//
//
//  The contestants are free to use these functions as-is or make
//  modifications. If the contestants choose to use the provided
//  code, they are responsible for making sure that it works as
//  expected.
//
//  The code provided here has no real or implied warranties.
//
//
////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <limits>
#include <cassert>
#include <cctype>
#include <map>
#include <set>
#include "parser_helper.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

using namespace std;
struct mycellstruct
{
    int  successors;
    int * predecessors;
       
};

bool is_special_char (char c) {

  static const char specialChars[] = {'(', ')', ',', ':', ';', '/', '#', '[', ']', '{', '}', '*', '\"', '\\'} ;
  
  for (int i=0; i < sizeof(specialChars); ++i) {
    if (c == specialChars[i])
      return true ;
  }

  return false ;
}

// Read the next line and return it as a list of tokens skipping white space and special characters
// The return value indicates success/failure.
bool read_line_as_tokens (istream& is, vector<string>& tokens,
			  bool includeSpecialChars = false) {

  tokens.clear() ;
  
  string line ;
  std::getline (is, line) ;

  while (is && tokens.empty()) {
    
    string token = "" ;

    for (int i=0; i < line.size(); ++i) {
      char currChar = line[i] ;
      bool isSpecialChar = is_special_char(currChar) ;

      if (std::isspace (currChar) || isSpecialChar) {

        if (!token.empty()) {
          // Add the current token to the list of tokens
          tokens.push_back(token) ;
          token.clear() ;
        }

	if (includeSpecialChars && isSpecialChar) {
	  tokens.push_back(string(1, currChar)) ;
	}

      } else {
        // Add the char to the current token
        token.push_back(currChar) ;
      }

    }

    if (!token.empty())
      tokens.push_back(token) ;
    

    if (tokens.empty())
      // Previous line read was empty. Read the next one.
      std::getline (is, line) ;    
  }

  //for (int i=0; i < tokens.size(); ++i)
  //  cout << tokens[i] << " " ;
  //cout << endl ;
  
  return !tokens.empty() ;
}



bool VerilogParser::read_module (string& moduleName) {

  vector<string> tokens ;
  bool valid = read_line_as_tokens (is, tokens) ;

  while (valid) {

    if (tokens.size() == 2 && tokens[0] == "module") {
      moduleName = tokens[1] ;

      break ;
    }

    valid = read_line_as_tokens (is, tokens) ;
  }

  // Read and skip the port names in the module definition
  // until we encounter the tokens {"Start", "PIs"}
  while (valid && !(tokens.size() == 2 && tokens[0] == "Start" && tokens[1] == "PIs")) {

    valid = read_line_as_tokens (is, tokens) ;
    assert (valid) ;
  }

  return valid ;
}

bool VerilogParser::read_primary_input (string& primaryInput) {

  primaryInput = "" ;
  
  vector<string> tokens ;
  bool valid = read_line_as_tokens (is, tokens) ;

  assert (valid) ;
  assert (tokens.size() == 2) ;

  if (valid && tokens[0] == "input") {
    primaryInput = tokens[1] ;

  } else {
    assert (tokens[0] == "Start" && tokens[1] == "POs") ;
    return false ;
  }

  return valid ; 
}



bool VerilogParser::read_primary_output (string& primaryOutput) {

  primaryOutput = "" ;
  
  vector<string> tokens ;
  bool valid = read_line_as_tokens (is, tokens) ;

  assert (valid) ;
  assert (tokens.size() == 2) ;

  if (valid && tokens[0] == "output") {
    primaryOutput = tokens[1] ;

  } else {
    assert (tokens[0] == "Start" && tokens[1] == "wires") ;
    return false ;
  }

  return valid ; 
}


bool VerilogParser::read_wire (string& wire) {

  wire = "" ;
  
  vector<string> tokens ;
  bool valid = read_line_as_tokens (is, tokens) ;

  assert (valid) ;
  assert (tokens.size() == 2) ;

  if (valid && tokens[0] == "wire") {
    wire = tokens[1] ;

  } else {
    assert (tokens[0] == "Start" && tokens[1] == "cells") ;
    return false ;
  }

  return valid ; 
}


bool VerilogParser::read_cell_inst (string& cellType, string& cellInstName,
                                    vector<std::pair<string, string> >& pinNetPairs) {

  cellType = "" ;
  cellInstName = "" ;
  pinNetPairs.clear() ;

  vector<string> tokens ;
  bool valid = read_line_as_tokens (is, tokens) ;

  assert (valid) ;

  if (tokens.size() == 1) {
    assert (tokens[0] == "endmodule") ;
    return false ;
  }

  assert (tokens.size() >= 4) ; // We should have cellType, instName, and at least one pin-net pair

  cellType = tokens[0] ;
  cellInstName = tokens[1] ;

  for (int i=2; i < tokens.size()-1; i += 2) {

    assert (tokens[i][0] == '.') ; // pin names start with '.'
    string pinName = tokens[i].substr(1) ; // skip the first character of tokens[i] 
    
    pinNetPairs.push_back(std::make_pair(pinName, tokens[i+1])) ;
  }

  return valid ;
}


// Read clock definition
// Return value indicates if the last read was successful or not.  
bool SdcParser::read_clock (string& clockName, string& clockPort, double& period) {

  clockName = "" ;
  clockPort = "" ;
  period = 0.0 ;
  
  vector<string> tokens ;
  bool valid = read_line_as_tokens (is, tokens) ;

  while (valid) {

    if (tokens.size() == 7 && tokens[0] == "create_clock" && tokens[1] == "-name") {

      clockName = tokens[2] ;
  
      assert (tokens[3] == "-period") ;
      period = std::atof(tokens[4].c_str()) ;
      
      assert (tokens[5] == "get_ports") ;
      clockPort = tokens[6] ;
      break ;
    }

    valid = read_line_as_tokens (is, tokens) ;    
  }
  
  // Skip the next comment line to prepare for the next stage
  bool valid2 = read_line_as_tokens (is, tokens) ;
  assert (valid2) ;
  assert (tokens.size() == 2) ;
  assert (tokens[0] == "input" && tokens[1] == "delays") ;
  
  return valid ;
}

// Read input delay
// Return value indicates if the last read was successful or not.
bool SdcParser::read_input_delay (string& portName, double& delay) {

  portName = "" ;
  delay = 0.0 ;

  vector<string> tokens ;
  bool valid = read_line_as_tokens (is, tokens) ;

  assert (valid) ;
  assert (tokens.size() >= 2) ;

  if (valid && tokens[0] == "set_input_delay") {
    assert (tokens.size() == 6) ;

    delay = std::atof(tokens[1].c_str()) ;

    assert (tokens[2] == "get_ports") ;

    portName = tokens[3] ;

    assert (tokens[4] == "-clock") ;

  } else {

    assert (tokens.size() == 2) ;
    assert (tokens[0] == "input" && tokens[1] == "drivers") ;

    return false ;
    
  }

  return valid ;
}


// Read output delay
// Return value indicates if the last read was successful or not.
bool SdcParser::read_output_delay (string& portName, double& delay) {

  portName = "" ;
  delay = 0.0 ;

  vector<string> tokens ;
  bool valid = read_line_as_tokens (is, tokens) ;

  assert (valid) ;
  assert (tokens.size() >= 2) ;

  if (valid && tokens[0] == "set_output_delay") {
    assert (tokens.size() == 6) ;

    delay = std::atof(tokens[1].c_str()) ;

    assert (tokens[2] == "get_ports") ;

    portName = tokens[3] ;

    assert (tokens[4] == "-clock") ;

  } else {

    assert (tokens.size() == 2) ;
    assert (tokens[0] == "output" && tokens[1] == "loads") ;

    return false ;
    
  }

  return valid ;
}


// Read driver info for the input port
// Return value indicates if the last read was successful or not.
bool SdcParser::read_driver_info (string& inPortName, string& driverSize, string& driverPin,
                                  double& inputTransitionFall, double& inputTransitionRise) {

  inPortName = "" ;
  driverSize = "" ;
  driverPin = "" ;
  inputTransitionFall = 0.0 ;
  inputTransitionRise = 0.0 ;
  
  vector<string> tokens ;
  bool valid = read_line_as_tokens (is, tokens) ;

  assert (valid) ;
  assert (tokens.size() >= 2) ;

  if (valid && tokens[0] == "set_driving_cell") {
    assert (tokens.size() == 11) ;
    assert (tokens[1] == "-lib_cell") ;

    driverSize = tokens[2] ;

    assert (tokens[3] == "-pin") ;
    driverPin = tokens[4] ;

    assert (tokens[5] == "get_ports") ;
    inPortName = tokens[6] ;

    assert (tokens[7] == "-input_transition_fall") ;
    inputTransitionFall = std::atof(tokens[8].c_str()) ;

    assert (tokens[9] == "-input_transition_rise") ;
    inputTransitionRise = std::atof(tokens[10].c_str()) ;    

  } else {

    assert (tokens.size() == 2) ;
    assert (tokens[0] == "output" && tokens[1] == "delays") ;

    return false ;
  }

  return valid ;
}



// Read output load
// Return value indicates if the last read was successful or not.  
bool SdcParser::read_output_load (string& outPortName, double& load) {

  outPortName = "" ;
  load = 0.0 ;

  vector<string> tokens ;
  bool valid = read_line_as_tokens (is, tokens) ;

  if (valid && tokens[0] == "set_load") {
    assert (tokens.size() == 5) ;

    assert (tokens[1] == "-pin_load") ;
    load = std::atof(tokens[2].c_str()) ;

    assert (tokens[3] == "get_ports") ;
    outPortName = tokens[4] ;

  } else {

    assert (!valid) ;
    return false ;
  }

  return valid ;
}

// The return value indicates whether the *CONN section has been read or not
bool SpefParser::read_connections (vector<SpefConnection>& connections) {

  connections.clear() ; // in case the input is not empty
  bool terminateEarly = false ;

  vector<string> tokens ;
  bool valid = read_line_as_tokens (is, tokens, true /*include special chars*/) ;
  
  // Skip the lines that are not "*CONN"
  while (valid && !(tokens.size() == 2 && tokens[0] == "*" && tokens[1] == "CONN")) {

    // The following if condition checks for nets without any connections
    // This is needed for clock nets.
    if (tokens.size() == 2 && tokens[0] == "*" && tokens[1] == "END") {
      terminateEarly = true ;
      break ;
    }

    valid = read_line_as_tokens (is, tokens, true /*include special chars*/) ;
  }

  assert (valid) ; // end of file not expected here

  if (terminateEarly)
    return false ;

  while (valid) {
    valid = read_line_as_tokens (is, tokens, true /*include special chars*/) ;

    if (tokens.size() == 2 && tokens[0] == "*" && tokens[1] == "CAP")
      break ; // the beginning of the next section

    // Line format: "*nodeType nodeName direction"    
    // Note that nodeName can be either a single token or 3 tokens

    assert (tokens.size() == 4 || tokens.size() == 6) ; 
    assert (tokens[0] == "*") ;
    
    SpefConnection curr ;
    int tokenIndex = 1 ;

    assert (tokens[tokenIndex].size() == 1) ; // should be a single character
    curr.nodeType = tokens[tokenIndex++][0] ;
    assert (curr.nodeType == 'P' || curr.nodeType == 'I') ;

    curr.nodeName.n1 = tokens[tokenIndex++] ;
    if (tokens[tokenIndex] == ":") {
      ++tokenIndex ; // skip the current token
      curr.nodeName.n2 = tokens[tokenIndex++] ;
    }

    assert (tokens[tokenIndex].size() == 1) ; // should be a single character
    curr.direction = tokens[tokenIndex++][0] ;
    assert (curr.direction == 'I' || curr.direction == 'O') ;
    
    connections.push_back(curr) ;
  }

  return true ;
}

void SpefParser::read_capacitances (vector<SpefCapacitance>& capacitances) {

  capacitances.clear() ; // in case the input is not empty

  vector<string> tokens ;
  bool valid = true ;
  while (valid) {
    
    valid = read_line_as_tokens (is, tokens, true /*include special chars*/) ;
    
    if (tokens.size() == 2 && tokens[0] == "*" && tokens[1] == "RES") 
      break ; // the beginning of the next section

    // Line format: "index nodeName cap"
    // Note that nodeName can be either a single token or 3 tokens

    assert (tokens.size() == 3 || tokens.size() == 5) ;

    SpefCapacitance curr ;
    int tokenIndex = 1 ;
    
    curr.nodeName.n1 = tokens[tokenIndex++] ;
    if (tokens[tokenIndex] == ":") {
      ++tokenIndex ; // skip the current token
      curr.nodeName.n2 = tokens[tokenIndex++] ;
    }
    
    curr.capacitance = std::atof(tokens[tokenIndex++].c_str()) ;
    assert (curr.capacitance >= 0) ;

    capacitances.push_back(curr) ;
  }
}

void SpefParser::read_resistances (vector<SpefResistance>& resistances) {

  resistances.clear() ; // in case the input is not empty

  vector<string> tokens ;
  bool valid = true ;
  while (valid) {
    
    valid = read_line_as_tokens (is, tokens, true /*include special chars*/) ;
    
    if (tokens.size() == 2 && tokens[0] == "*" && tokens[1] == "END") 
      break ; // end for this net

    // Line format: "index fromNodeName toNodeName res"
    // Note that each nodeName can be either a single token or 3 tokens

    assert (tokens.size() >= 4 && tokens.size() <= 8) ;

    SpefResistance curr ;
    int tokenIndex = 1 ;
    
    curr.fromNodeName.n1 = tokens[tokenIndex++] ;
    if (tokens[tokenIndex] == ":") {
      ++tokenIndex ; // skip the current token
      curr.fromNodeName.n2 = tokens[tokenIndex++] ;
    }

    curr.toNodeName.n1 = tokens[tokenIndex++] ;
    if (tokens[tokenIndex] == ":") {
      ++tokenIndex ; // skip the current token
      curr.toNodeName.n2 = tokens[tokenIndex++] ;
    }
    
    curr.resistance = std::atof(tokens[tokenIndex++].c_str()) ;
    assert (curr.resistance >= 0) ;

    resistances.push_back(curr) ;
  }
}

// Read the spef data for the next net.
// Return value indicates if the last read was successful or not.  
bool SpefParser::read_net_data (SpefNet& spefNet) {

  spefNet.clear() ;

  vector<string> tokens ;
  bool valid = read_line_as_tokens (is, tokens, true /*include special chars*/) ;

  // Read until a valid D_NET line is found
  while (valid) {
    if (tokens.size() == 4 && tokens[0] == "*" && tokens[1] == "D_NET") {
      spefNet.netName = tokens[2] ;
      spefNet.netLumpedCap = std::atof(tokens[3].c_str()) ;

      bool readConns = read_connections (spefNet.connections) ;
      if (readConns) {
	read_capacitances (spefNet.capacitances) ;
	read_resistances (spefNet.resistances) ;
      }

      return true ;
    }

    valid = read_line_as_tokens (is, tokens, true /*include special chars*/) ;
  }

  return false ; // a valid net was not read
}


// Read timing info for the next pin or port
// Return value indicates if the last read was successful or not.  
// If the line read corresponds to a pin, then name1 and name2 will be set to the cell 
// instance name and the pin name, respectively.
// If the line read corresponds to a port, then name1 will be set to the port name, and
// name2 will be set to "".
bool TimingParser::read_timing_line (string& name1, string& name2,
				     double& riseSlack, double& fallSlack,
				     double& riseTransition, double& fallTransition,
				     double& riseArrival, double& fallArrival) {

  vector<string> tokens ;
  bool valid = read_line_as_tokens (is, tokens) ;

  if (!valid)
    return false ;

  assert (tokens.size() >= 7) ;

  int tokenIndex = 0 ;
  name1 = tokens[tokenIndex++] ;
  name2 = "" ; 
  if (tokens.size() == 8) {
    // line corresponds to a cell pin
    name2 = tokens[tokenIndex++] ; 
  }

  riseSlack = std::atof(tokens[tokenIndex++].c_str()) ;
  fallSlack = std::atof(tokens[tokenIndex++].c_str()) ;
  riseTransition = std::atof(tokens[tokenIndex++].c_str()) ;
  fallTransition = std::atof(tokens[tokenIndex++].c_str()) ;
  riseArrival = std::atof(tokens[tokenIndex++].c_str()) ;
  fallArrival = std::atof(tokens[tokenIndex++].c_str()) ;

  return true ;
  
}


// Read ceff values for the next pin or port
// Return value indicates if the last read was successful or not.  
// If the line read corresponds to a pin, then name1 and name2 will be set to the cell 
// instance name and the pin name, respectively.
// If the line read corresponds to a port, then name1 will be set to the port name, and
// name2 will be set to "".
bool CeffParser::read_ceff_line (string& name1, string& name2, double& riseCeff, double& fallCeff) {


  vector<string> tokens ;
  bool valid = read_line_as_tokens (is, tokens) ;

  if (!valid)
    return false ;

  assert (tokens.size() >= 3) ;

  int tokenIndex = 0 ;
  name1 = tokens[tokenIndex++] ;
  name2 = "" ; 
  if (tokens.size() == 4) {
    // line corresponds to a cell pin
    name2 = tokens[tokenIndex++] ; 
  }

  riseCeff = std::atof(tokens[tokenIndex++].c_str()) ;
  fallCeff = std::atof(tokens[tokenIndex++].c_str()) ;

  return true ;
}


// No need to parse the 3D LUTs, because they will be ignored
void LibParser::_skip_lut_3D () {

  std::vector<string> tokens ;
  
  bool valid = read_line_as_tokens (is, tokens) ;
  assert (valid) ;
  assert (tokens[0] == "index_1") ;
  assert (tokens.size() >= 2) ;
  int size1 = tokens.size() - 1 ;

  valid = read_line_as_tokens (is, tokens) ;
  assert (valid) ;
  assert (tokens[0] == "index_2") ;
  assert (tokens.size() >= 2) ;
  int size2 = tokens.size() - 1 ;  

  valid = read_line_as_tokens (is, tokens) ;
  assert (valid) ;
  assert (tokens[0] == "index_3") ;
  assert (tokens.size() >= 2) ;
  int size3 = tokens.size() - 1 ;  

  valid = read_line_as_tokens (is, tokens) ;
  assert (valid) ;
  assert (tokens.size() == 1 && tokens[0] == "values") ;
  
  for (int i=0; i < size1; ++i) {
    for (int j=0; j < size2; ++j) {

      valid = read_line_as_tokens (is, tokens) ;
      assert (valid) ;
      assert (tokens.size() == size3) ;
    }
  }

}

void LibParser::_begin_read_lut (LibParserLUT& lut) {

  std::vector<string> tokens ;
  bool valid = read_line_as_tokens (is, tokens) ;

  assert (valid) ;
  assert (tokens[0] == "index_1") ;
  assert (tokens.size() >= 2) ;
  
  int size1 = tokens.size()-1 ;
  lut.loadIndices.resize(size1) ;
  for (int i=0; i < tokens.size()-1; ++i) {
    
    lut.loadIndices[i] = std::atof(tokens[i+1].c_str()) ;
  }

  valid = read_line_as_tokens (is, tokens) ;

  assert (valid) ;
  assert (tokens[0] == "index_2") ;
  assert (tokens.size() >= 2) ;

  int size2 = tokens.size()-1 ;
  lut.transitionIndices.resize(size2) ;
  for (int i=0; i < tokens.size()-1; ++i) {
    
    lut.transitionIndices[i] = std::atof(tokens[i+1].c_str()) ;
  }
  
  valid = read_line_as_tokens (is, tokens) ;
  assert (valid) ;
  assert (tokens.size() == 1 && tokens[0] == "values") ;

  lut.tableVals.resize(size1) ;
  for (int i=0 ; i < lut.loadIndices.size(); ++i) {
    valid = read_line_as_tokens (is, tokens) ;
    assert (valid) ;
    assert (tokens.size() == lut.transitionIndices.size()) ;

    lut.tableVals[i].resize(size2) ;
    for (int j=0; j < lut.transitionIndices.size(); ++j) {
      lut.tableVals[i][j] = std::atof(tokens[j].c_str()) ;

    }
  }

  
}

void LibParser::_begin_read_timing_info (string toPin, LibParserTimingInfo& timing) {

  timing.toPin = toPin ;
  
  bool finishedReading = false ;

  std::vector<string> tokens ;  
  while (!finishedReading) {

    bool valid = read_line_as_tokens (is, tokens) ;
    assert (valid) ;
    assert (tokens.size() >= 1) ;

    if (tokens[0] == "cell_fall") {
      _begin_read_lut (timing.fallDelay) ;

    } else if (tokens[0] == "cell_rise") {
      _begin_read_lut (timing.riseDelay) ;

    } else if (tokens[0] == "fall_transition") {
      _begin_read_lut (timing.fallTransition) ;
      
    } else if (tokens[0] == "rise_transition") {
      _begin_read_lut (timing.riseTransition) ;

    } else if (tokens[0] == "fall_constraint") {

      _skip_lut_3D() ; // will ignore fall constraints      

    } else if (tokens[0] == "rise_constraint") {

      _skip_lut_3D() ; // will ignore rise constraints

    } else if (tokens[0] == "timing_sense") {
      timing.timingSense = tokens[1] ;

    } else if (tokens[0] == "related_pin") {

      assert (tokens.size() == 2) ;
      timing.fromPin = tokens[1] ;

    } else if (tokens[0] == "End") {

      assert (tokens.size() == 2) ;
      assert (tokens[1] == "timing") ;
      finishedReading = true ;
      
    } else if (tokens[0] == "timing_type") {
      // ignore data

    } else if (tokens[0] == "related_output_pin") {
      // ignore data

    } else {

      cout << "Error: Unknown keyword: " << tokens[0] << endl ;
      assert (false) ; // unknown keyword
    }
    
  }


}


void LibParser::_begin_read_pin_info (string pinName, LibParserCellInfo& cell, LibParserPinInfo& pin) {

  pin.name = pinName ;
  pin.isClock = false ;
  pin.maxCapacitance = std::numeric_limits<double>::max() ;
  
  bool finishedReading = false ;

  std::vector<string> tokens ;  
  while (!finishedReading) {

    bool valid = read_line_as_tokens (is, tokens) ;
    assert (valid) ;
    assert (tokens.size() >= 1) ;

    if (tokens[0] == "direction") {

      assert (tokens.size() == 2) ;
      if (tokens[1] == "input")
        pin.isInput = true ;
      else if (tokens[1] == "output")
        pin.isInput = false ;
      else
        assert (false) ; // undefined direction

    } else if (tokens[0] == "capacitance") {

      assert (tokens.size() == 2) ;
      pin.capacitance = std::atof(tokens[1].c_str()) ;

    } else if (tokens[0] == "max_capacitance") {

      assert (tokens.size() == 2) ;
      pin.maxCapacitance = std::atof(tokens[1].c_str()) ;


    } else if (tokens[0] == "timing") {

      cell.timingArcs.push_back(LibParserTimingInfo()) ; // add an empty TimingInfo object
      _begin_read_timing_info (pinName, cell.timingArcs.back()) ; // pass the empty object to the function to be filled

    } else if (tokens[0] == "clock") {

      pin.isClock = true ;

    } else if (tokens[0] == "End") {

      assert (tokens.size() == 2) ;
      assert (tokens[1] == "pin") ;
      finishedReading = true ;

    } else if (tokens[0] == "function") {

      // ignore data

    } else if (tokens[0] == "min_capacitance") {

      // ignore data

    } else if (tokens[0] == "nextstate_type") {

      // ignore data

    } else {
      cout << "Error: Unknown keyword: " << tokens[0] << endl ;      
      assert (false) ; // unknown keyword 

    }

  }


}

void LibParser::_begin_read_cell_info (string cellName, LibParserCellInfo& cell) {

  cell.name = cellName ;
  cell.isSequential = false ;
  cell.dontTouch = false ;
  
  bool finishedReading = false ;

  std::vector<string> tokens ;  
  while (!finishedReading) {

    bool valid = read_line_as_tokens (is, tokens) ;
    assert (valid) ;
    assert (tokens.size() >= 1) ;
    
    if (tokens[0] == "cell_leakage_power") {

      assert (tokens.size() == 2) ;
      cell.leakagePower = std::atof(tokens[1].c_str()) ;

    } else if (tokens[0] == "cell_footprint") {
        
      assert (tokens.size() == 2) ;
      cell.footprint = tokens[1] ;

    } else if (tokens[0] == "area") {

      assert (tokens.size() == 2) ;
      cell.area = std::atof(tokens[1].c_str()) ;

    } else if (tokens[0] == "clocked_on") {

      cell.isSequential = true ;

    } else if (tokens[0] == "dont_touch") {

      cell.dontTouch = true ;
      
    } else if (tokens[0] == "pin") {

      assert (tokens.size() == 2) ;

      cell.pins.push_back(LibParserPinInfo()) ; // add empty PinInfo object
      _begin_read_pin_info (tokens[1], cell, cell.pins.back()) ; // pass the new PinInfo object to be filled

    } else if (tokens[0] == "End") {

      assert (tokens.size() == 3) ;
      assert (tokens[1] == "cell") ;
      assert (tokens[2] == cellName) ;
      finishedReading = true ;

    } else if (tokens[0] == "cell_footprint") {

      // ignore data

    } else if (tokens[0] == "ff") {

      // ignore data

    } else if (tokens[0] == "next_state") {

      // ignore data

    } else if (tokens[0] == "dont_use") {

      // ignore data
      
    } else {

      cout << "Error: Unknown keyword: " << tokens[0] << endl ;
      assert (false) ; // unknown keyword
    }
  } 

}


// Read the default max_transition defined for the library.
// Return value indicates if the last read was successful or not.  
// This function must be called in the beginning before any read_cell_info function call.
bool LibParser::read_default_max_transition (double& maxTransition) {

  maxTransition = 0.0 ;
  vector<string> tokens ;

  bool valid = read_line_as_tokens (is, tokens) ;

  while (valid) {

    if (tokens.size() == 2 && tokens[0] == "default_max_transition") {
      maxTransition = std::atof(tokens[1].c_str()) ;
      return true ;
    }

    valid = read_line_as_tokens (is, tokens) ;
  }

  return false ;
}



// Read the next standard cell definition.
// Return value indicates if the last read was successful or not.  
bool LibParser::read_cell_info (LibParserCellInfo& cell) {

  vector<string> tokens ;
  bool valid = read_line_as_tokens (is, tokens) ;


  while (valid) {

    if (tokens.size() == 2 && tokens[0] == "cell") {
      _begin_read_cell_info (tokens[1], cell) ;

      return true ;
    }

    valid = read_line_as_tokens (is, tokens) ;
  }

  return false ;
}

ostream& operator<< (ostream& os, LibParserLUT& lut) {

  if (lut.loadIndices.empty() && lut.transitionIndices.empty() && lut.tableVals.empty())
    return os ;

    // We should have either all empty or none empty.
  assert (!lut.loadIndices.empty() && !lut.transitionIndices.empty() && !lut.tableVals.empty()) ;
  
  assert (lut.tableVals.size() == lut.loadIndices.size()) ;
  assert (lut.tableVals[0].size() == lut.transitionIndices.size()) ;

  cout << "\t" ;
  for (int i=0; i < lut.transitionIndices.size(); ++i) {
    cout << lut.transitionIndices[i] << "\t" ;
  }
  cout << endl ;

  
  for (int i=0; i < lut.loadIndices.size(); ++i) {
    cout << lut.loadIndices[i] << "\t" ;

    for (int j=0; j < lut.transitionIndices.size(); ++j)
      cout << lut.tableVals[i][j] << "\t" ;

    cout << endl ;
    
  }

  return os ;
}


ostream& operator<< (ostream& os, LibParserTimingInfo& timing) {

  cout << "Timing info from " << timing.fromPin << " to " << timing.toPin << ": " << endl ;
  cout << "Timing sense: " << timing.timingSense << endl ;

  cout << "Fall delay LUT: " << endl ;
  cout << timing.fallDelay ;

  cout << "Rise delay LUT: " << endl ;
  cout << timing.riseDelay ;
  
  cout << "Fall transition LUT: " << endl ;
  cout << timing.fallTransition ;

  cout << "Rise transition LUT: " << endl ;
  cout << timing.riseTransition ;

  return os ;
}


ostream& operator<< (ostream& os, LibParserPinInfo& pin) {

  cout << "Pin " << pin.name << ":" << endl ;
  cout << "capacitance: " << pin.capacitance << endl ;
  cout << "maxCapacitance: " << pin.maxCapacitance << endl ;
  cout << "isInput? " << (pin.isInput ? "true" : "false") << endl ;
  cout << "isClock? " << (pin.isClock ? "true" : "false") << endl ;
  cout << "End pin" << endl ;

  return os ;
}


ostream& operator<< (ostream& os, LibParserCellInfo& cell) {

  cout << "Library cell " << cell.name << ": " << endl ;

  cout << "Footprint: " << cell.footprint << endl ;
  cout << "Leakage power: " << cell.leakagePower << endl ;
  cout << "Area: " << cell.area << endl ;
  cout << "Sequential? " << (cell.isSequential ? "yes" : "no") << endl ;
  cout << "Dont-touch? " << (cell.dontTouch ? "yes" : "no") << endl ;      

  cout << "Cell has " << cell.pins.size() << " pins: " << endl ;
  for (int i=0; i < cell.pins.size(); ++i) {
    cout << cell.pins[i] << endl ;
  }

  cout << "Cell has " << cell.timingArcs.size() << " timing arcs: " << endl ;
  for (int i=0; i < cell.timingArcs.size(); ++i) {
    cout << cell.timingArcs[i] << endl ;
  }

  cout << "End of cell " << cell.name << endl << endl ;

  return os ;
}



// Example function that uses VerilogParser class to parse the given ISPD-13 verilog
// file. The extracted data is simply printed out in this example.
void test_verilog_parser (string filename,string format) {
  set<std::string> piset; // all the pis are added to this set
  set<std::string> poset;// all the pos are added to this set
  set<std::string> wireset;// all the wires are added to this set
  set<std::string>::iterator myit;// this is an iterator that iterates through the set can be used for piset,poset or wireset
   map<int,std::string> mymap;// a map where number is the key and value is the component either pi,po or the cell
   set<std::string> clkset;// added on 15th july to treat clocks screws up slack estimation
   map<std::string,int> mymap2;//a map where the component is the key and value is the number
   map<int,std::string>::iterator mapit;// an iterator that iterates through the map
   map<int,std::string>::iterator mapit2;// a second iterator that iterates through the map used for file writes

   map<int,vector<string> > cellinputmap;// if a wire ( not pi not po) is in the input of a cell it gets added to this map. 
   map<int,vector<string> >celloutputmap;// if a wire (not pi not po) is in the output of a cell it gets added to this map. Both cellinput map and celloutputmap are useful for adding edges b/w cells
 typedef boost::adjacency_list<boost::vecS,boost::vecS,boost::directedS> Graph; // we intialize a graph ,for the time being it is bidirectional although making it uni-directional or a DAG will not make any difference because we are not adding any backward edges all the edges are forward (from PI->PO)

  VerilogParser vp (filename) ;
  int elementcount=0;// counts the pis and pos alone
  int totalcount=0;//counts total components pi+po+cells used as key for hash map mymap line no 1116
  string moduleName ;
  bool valid = vp.read_module (moduleName) ;
  assert (valid) ;
  
  cout << "Module " << moduleName << endl << endl ;

  do {
    string primaryInput ;
    valid = vp.read_primary_input (primaryInput) ;

    if (valid)
    {
      cout << "Primary input: " << primaryInput << endl ;
      if(primaryInput=="ispd_clk")
          clkset.insert(primaryInput);
      else
      {
          piset.insert(primaryInput); // we add primary input to the set of PIS
      mymap[totalcount]=primaryInput;// add it to the hash map and increment elementcount and totalcount
      mymap2[primaryInput]=totalcount;
      elementcount++;
      totalcount++;
      }
    }
  } while (valid) ;

  cout << endl ;

  do {
    string primaryOutput ;
    valid = vp.read_primary_output (primaryOutput) ;

    if (valid)
    {
      cout << "Primary output: " << primaryOutput << endl ;
      poset.insert(primaryOutput);// we add the primary output to the set of POs
      mymap[totalcount]=primaryOutput;// add it to the hash map mymap and increment elementcount and totalcount
      mymap2[primaryOutput]=totalcount;
      elementcount++;
      totalcount++;
    }

  } while (valid) ;
//  elementcount--;
  cout << endl ;

  do {
    string net ;
    valid = vp.read_wire (net) ;

    if (valid)
    {
        
    
          cout << "Net: " << net << endl ;
          wireset.insert(net);//if the net is not a pi nor a po then it will be added to wireset which is a set of intermediated wires
          
          
        
    }

  } while (valid) ;
    Graph g;
  cout<<"total count"<<totalcount<<endl;
  int cellcount=0;// a variable that counts only the number of cells
  cout << endl ;
  cout << "Cell insts: " << std::endl ;
//   cellcount++;
  

  do {
    string cellType, cellInst ;// each cell comes with a celltype and cellInst  we use cellType to index into the hashmap as it will be easy to get the cell details like power area and timing during library parsing
    vector<std::pair<string, string> > pinNetPairs ;
    vector<string> tempinvect;    // a vector that stores all the intermediate wires that feed into the cell
    vector<string>tempoutvect; // a vector that stores all the intermediate wires that are the cell's output
    valid = vp.read_cell_inst (cellType, cellInst, pinNetPairs) ;
    int temp=0;
    if (valid) {
      cout << cellType << " " << elementcount+cellcount << " "<<totalcount<<" " ;
      
      mymap[totalcount]=cellType;// we add the cellType to the hashmap instead of the cellInst as cellType will be useful for getting cell details like power area and timing during library parsing.
      mymap2[cellType]=totalcount;//find out where are u using this again
      
      int sizecount=pinNetPairs.size();
            
      for (int i=0; i < pinNetPairs.size(); ++i) {

           if(piset.find(pinNetPairs[i].second)!=piset.end()) // if the pin is the primary input set then
       {
      
            temp=mymap2[pinNetPairs[i].second]; 
          cout<<"primary input "<<pinNetPairs[i].second<<" "<<temp<<" ";

           boost::add_edge(temp,totalcount,g); // we add an incoming edge from PI to the cell
       }
           else if(clkset.find(pinNetPairs[i].second)!=clkset.end())
           {
               cout<<"clk "<<pinNetPairs[i].second<<" ";
               temp=mymap2[pinNetPairs[i].second];
//               boost::add_edge(temp,totalcount,g);
           }
       else if(pinNetPairs[i].first=="o" && poset.find(pinNetPairs[i].second)!=poset.end()) // if the pin is the primary output set then
       {
//      mapit=mymap.begin();
            temp=mymap2[pinNetPairs[i].second];
          cout<<"primary output "<<pinNetPairs[i].second<<" "<<temp<<" ";

           boost::add_edge(totalcount,temp,g);// we add an from the cell to the PO
        
       }
      if(wireset.find(pinNetPairs[i].second)!=wireset.end())// if it is an intermediate wire
      {
          if(pinNetPairs[i].first=="o"){ // if it is a output of the cell
              
              tempoutvect.push_back(pinNetPairs[i].second); // we add it to the temporary outputvector
             cout<<"output wire "<<pinNetPairs[i].second<<" ";
              sizecount--;
          }
      
       else 
       {
           cout<<"input wire "<<pinNetPairs[i].second<<" "; // else if it is an input of the cell
        tempinvect.push_back(pinNetPairs[i].second); // we add it to the temporary inputvector
      
       }
      }
        
            }
       cout << endl ;
       totalcount++;
   celloutputmap[elementcount+cellcount]=tempoutvect; // we map the temporary outputvector to the celloutputmap
  cellinputmap[elementcount+cellcount]=tempinvect;//we map the temporary inputvector the cellinputmap
  cellcount++;        
    }


    
  } while (valid) ;
    //cellcount--;
 vector<string> invect;// a temporary inputvector
 vector<string> outvect;// a temporary output vector
  vector<string>::iterator it1;// we use two vector iterators to iterate over the input and output vectors
  vector<string>::iterator it2;
string tempstring1,tempstring2;
cout<<"doing vector"<<endl;
  for(int j=0;j<cellcount;j++) // iterate over all the cells
{
    invect=cellinputmap[elementcount+j];// get the input vectors for the cell
    cout<<j<<"\t";
    for(int k=0;k<cellcount;k++)
    {
        if(j!=k)
        {
        outvect=celloutputmap[elementcount+k];// get the output vectors for the cell
        for(it1=invect.begin();it1<invect.end();it1++)
        {
            tempstring1=*it1;//for each input in cell1
            for(it2=outvect.begin();it2<outvect.end();it2++)
            {
                tempstring2=*it2;//for each output in cell2
                if(tempstring1==tempstring2)//if input matches output
                {
                    boost::add_edge(elementcount+k,elementcount+j,g);// add edge b/w cell1 and cell2
                }
            }
        }
        }
    }
}
cout<<"writing to file"<<endl;
if(format=="ispd")// we have two format options one is ispd and the other is hypergraph 
{
            string file=moduleName;
            string str2=file.append(".ispd");
                ofstream myfile;
                myfile.open(str2.c_str());
     
 /*
  * This is style 1 ispd benchmark format but multiple instances of same gate so have to use style 2*/
                /*       for(int i=0;i<elementcount+cellcount;i++)
{
    myfile<<mymap[i]<<" s 1\n";
    for(int j=0;j<elementcount+cellcount;j++)
    {
        if(boost::edge(i,j,g).second)
        {
            myfile<<mymap[j]<<"\n";
        }
    }
  //  myfile<<"\n";
}*/
//style 2  
for(int i=0;i<elementcount+cellcount;i++)
{

    if(poset.find(mymap[i])==poset.end())
        myfile<<i<<" s 1\n";// if component is not PO 
    for(int j=0;j<elementcount+cellcount;j++)
    {
        if(boost::edge(i,j,g).second) // if there exists an outgoing edge then write output to file
        {
            myfile<<j<<"\n";
        }
    }
  //  myfile<<"\n";
}
myfile.close();

}
else if(format=="hgr") // writes into hypergraph format
{
            string file=moduleName;
            string str2=file.append(".hgr");
                ofstream myfile;
                myfile.open(str2.c_str());
     
 /*
  * This is style 1 ispd benchmark format but multiple instances of same gate so have to use style 2*/
                /*       for(int i=0;i<elementcount+cellcount;i++)
{
    myfile<<mymap[i]<<" s 1\n";
    for(int j=0;j<elementcount+cellcount;j++)
    {
        if(boost::edge(i,j,g).second)
        {
            myfile<<mymap[j]<<"\n";
        }
    }
  //  myfile<<"\n";
}*/
//style 2 
myfile<<totalcount<<"\n";
for(int i=0;i<totalcount;i++)
{

    for(int j=0;j<totalcount;j++)
    {
        if(boost::edge(i,j,g).second)
        {
            myfile<<j<<"\t";
        }
    }
   myfile<<"\n";
}
myfile.close();
ofstream myfile4;
myfile4.open("debugcells.txt");
for(int i=0;i<totalcount;i++)
{
    myfile4<<mymap[i]<<"\t"<<i<<"\t";
    for(int j=0;j<totalcount;j++)
    {
        if(boost::edge(i,j,g).second)
        {
            myfile4<<mymap[j]<<"\t"<<j<<"\t";
        }
    }

    myfile4<<"\n";
}
}
ofstream myfile3;
myfile3.open("debug.txt");
for(int i=0;i<totalcount;i++)
{
    myfile3<<i<<"\t";
    for(int j=0;j<totalcount;j++)
    {
        if(boost::edge(j,i,g).second)
        {
            myfile3<<j<<"\t";
        }
    }
    myfile3<<"\n";
}
myfile3.close();
ofstream myfile2;
string s3=moduleName.append(".type");
myfile2.open(s3.c_str());
cout<<"key values"<<endl;
cout<<"the size is "<<mymap.size()<<endl;
//for(int i=0;i<elementcount+cellcount;i++)
mapit=mymap.begin();// we also create a type file which contains component,component TYPE,if it is a cell the cellType
do
{
    if(piset.find(mapit->second)!=piset.end())
    myfile2<<"PI\n";
    else if(clkset.find(mapit->second)!=clkset.end())
        myfile2<<"CLK\n";
    else if(poset.find(mapit->second)!=poset.end())
    myfile2<<"PO\n";
    else
        myfile2<<mapit->second<<"\n";
    ++mapit;
}while(mapit!=mymap.end());
myfile2.close();
//for(mapit=mymap.begin();mapit!=mymap.end();mapit++)
//{
//    if(poset.find(mapit->second)==poset.end())
//    myfile<<mapit->second<<" s 1\n";
//    for(mapit2=mymap.begin();mapit2!=mymap.end();mapit2++)
//    {
//        if(boost::edge(mapit->first,mapit2->second,g).second)
//        {
//            myfile<<mapit2->first<<"\n";
//        }
//    }
//}
//myfile.close();
  cout<<"Cellcount:"<<cellcount<<endl;
}

// Example function that uses SdcParser class to parse the given ISPD-13 sdc
// file. The extracted data is simply printed out in this example.
void test_sdc_parser (string filename) {

  SdcParser sp (filename) ;

  string clockName ;
  string clockPort ;
  double period ;
  bool valid = sp.read_clock (clockName, clockPort, period) ;

  assert(valid) ;
  cout << "Clock " << clockName << " connected to port " << clockPort
       << " has period " << period << endl ;

  do {
    string portName ;
    double delay ;

    valid = sp.read_input_delay (portName, delay) ;

    if (valid)
      cout << "Input port " << portName << " has delay " << delay << endl ;

  } while (valid) ;
  

  do {
    string portName ;
    string driverSize ;
    string driverPin ;
    double inputTransitionFall ;
    double inputTransitionRise ;
    
    valid = sp.read_driver_info (portName, driverSize, driverPin,
                                 inputTransitionFall, inputTransitionRise) ;

    if (valid) {
      cout << "Input port " << portName << " is assumed to be connected to the "
           << driverPin << " pin of lib cell " << driverSize << endl ;
      cout << "This virtual driver is assumed to have input transitions: "
           << inputTransitionFall << " (fall) and " << inputTransitionRise
           << " (rise)" << endl ;
    }
    
    
  } while (valid) ;

  do {
    string portName ;
    double delay ;

    valid = sp.read_output_delay (portName, delay) ;

    if (valid)
      cout << "Output port " << portName << " has delay " << delay << endl ;

  } while (valid) ;

  
  do {
    string portName ;
    double load ;

    valid = sp.read_output_load (portName, load) ;

    if (valid)
      cout << "Output port " << portName << " has load " << load << endl ;

  } while (valid) ;
  
  
}

// Example function that uses SpefParser class to parse the given ISPD-13 spef
// file. The extracted data is simply printed out in this example.
void test_spef_parser (string filename) {

  SpefParser sp (filename) ;

  SpefNet spefNet ;
  bool valid = sp.read_net_data (spefNet) ;

  int readCnt = 0 ;
  while (valid) {

    ++readCnt ;

    // print out the contents of the spefNet just read
    cout << "Net: " << spefNet.netName << endl ;
    cout << "Net lumped cap: " << spefNet.netLumpedCap << endl ;

    cout << "Connections: " << endl ;
    for (int i=0; i < spefNet.connections.size(); ++i) {
      cout << spefNet.connections[i] << endl ;
    }

    cout << "Capacitances: " << endl ;
    for (int i=0; i < spefNet.capacitances.size(); ++i) {
      cout << spefNet.capacitances[i] << endl ;
    }

    cout << "Resistances: " << endl ;
    for (int i=0; i < spefNet.resistances.size(); ++i) {
      cout << spefNet.resistances[i] << endl ;
    }

    cout << endl ;
    valid = sp.read_net_data (spefNet) ;
  }

  cout << "Read " << readCnt << " nets in the spef file." << endl ;
}

// Example function that uses TimingParser class to parse the given ISPD-13 timing
// file. The extracted data is simply printed out in this example.
void test_timing_parser (string filename) {

  TimingParser tp (filename) ;
  
  bool valid = false ;
  while (true) {
    
    string name1, name2 ;
    double riseSlack, fallSlack, riseTransition, fallTransition, riseArrival, fallArrival ;
    
    valid = tp.read_timing_line (name1, name2, riseSlack, fallSlack,
				 riseTransition, fallTransition, riseArrival, fallArrival) ;

    if (!valid)
      break ;

    if (name2 != "") {
      // timing info of a pin
      // name1: cellInstance, name2: pin
      std::cout << name1 << "/" << name2 << " "
		<< riseSlack << " " << fallSlack << " "
		<< riseTransition << " " << fallTransition << " "
		<< riseArrival << " " << fallArrival << endl ;
      
    } else {
      // timing of a port
      // name1: port name
      std::cout << name1 << " "
		<< riseSlack << " " << fallSlack << " "
		<< riseTransition << " " << fallTransition << " "
		<< riseArrival << " " << fallArrival << endl ;
    }
  }

}


// Example function that uses CeffParser class to parse the given ISPD-13 ceff
// file. The extracted data is simply printed out in this example.
void test_ceff_parser (string filename) {

  CeffParser tp (filename) ;
  
  bool valid = false ;
  while (true) {
    
    string name1, name2 ;
    double riseCeff, fallCeff ;
    
    valid = tp.read_ceff_line (name1, name2, riseCeff, fallCeff) ;

    if (!valid)
      break ;

    if (name2 != "") {
      // ceff values of a pin
      // name1: cellInstance, name2: pin
      std::cout << name1 << "/" << name2 << " "
		<< riseCeff << " " << fallCeff << endl ;
      
    } else {
      // timing of a port
      // name1: port name
      std::cout << name1 << " "
		<< riseCeff << " " << fallCeff << endl ;
    }
  }

}



// Example function that uses LibParser class to parse the given ISPD-13 lib
// file. The extracted data is simply printed out in this example.
void test_lib_parser (string filename) {

  LibParser lp (filename) ;

  double maxTransition = 0.0 ;
  bool valid = lp.read_default_max_transition(maxTransition) ;

  assert (valid) ;
  cout << "The default max transition defined is " << maxTransition << endl ;

  int readCnt = 0 ;
  do {
    LibParserCellInfo cell ;
    valid = lp.read_cell_info (cell) ;

    if (valid) {
      ++readCnt ;

      //cout << cell << endl ;
      cout << "Cell Name: " << cell.name << ", Cell Leakage Power: " << cell.leakagePower << endl;
    }
    
  } while (valid) ;

  cout << "Read " << readCnt << " number of library cells" << endl ;
}


int main(int args, char** argv) {

  if (args != 4) {
    cout << "Usage: " << argv[0] << " <format> <file> <outputtype:hgr ispd>" << endl ;
    exit(0) ;
  }

  if (argv[1] == string("verilog")) {
    cout << "Parsing verilog..." << endl ;
    test_verilog_parser(argv[2],argv[3]) ;
    cout << "Finished parsing verilog." << endl ;    
  } 
  else if (argv[1] == string("sdc")) {
    cout << "Parsing sdc..." << endl ;  
    test_sdc_parser (argv[2]) ;
    cout << "Finished parsing sdc." << endl ;
  }
  else if (argv[1] == string("spef")) {
    cout << "Parsing spef..." << endl ;  
    test_spef_parser (argv[2]) ;
    cout << "Finished parsing spef." << endl ;
  }
  else if (argv[1] == string("timing")) {
    cout << "Parsing timing..." << endl ;  
    test_timing_parser (argv[2]) ;
    cout << "Finished parsing timing." << endl ;
  }
  else if (argv[1] == string("ceff")) {
    cout << "Parsing ceff..." << endl ;  
    test_ceff_parser (argv[2]) ;
    cout << "Finished parsing ceff." << endl ;
  }
  else if (argv[1] == string("lib")) {
    cout << "Parsing lib..." << endl ;
    test_lib_parser (argv[2]) ;
    cout << "Finished parsing lib." << endl ;
  }
  else {
    cout << "Unknown format " << argv[1] << endl ;

  }
  
  return 0 ;
}
