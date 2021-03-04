#pragma once

#include "handlerPlugin.hh"
#include "stdafx.hh"
#include <thread>

typedef std::function<void (http_request&)> CMDFunctor;

class fsmTransition
{
    public:

    /**
       \brief Constructor

       \param istate initial state name
       \param fstate final state name
       \param f CMDfunctor called

       \details In an object of class MyObj with a methode create(http_request message), the CMDFunctor is declared with 'std::bind(&MyObj::create, this,std::placeholders_1)'

     */
  fsmTransition(std::string istate,std::string fstate,CMDFunctor f) : _istate(istate),_fstate(fstate),_callback(f) {}
    std::string initialState(){return _istate;} ///< Initial state name
    std::string finalState(){return _fstate;} ///< Final state name
    CMDFunctor callback(){return _callback;} ///< PFunctor access
    private:
      std::string _istate,_fstate;
      CMDFunctor _callback;
  };



class fsmw : public handlerPlugin
  {
  public:

    /**
       \brief Constructor
       \param dire is the directory path for writing
     */
    fsmw();
    

    /**
       \brief Start a run
       \param run is the run number
       \details It creates the file SMM_dayMonthYear_HourMinuteSeconde_run.dat
     */
    virtual std::vector<std::string> getPaths(std::string query);
    virtual void processRequest(http_request& message);
    virtual void terminate();
    // To be implemented on inheritance
    virtual void initialise();
    virtual void end();
    // FSMW web services
    void list(http_request message);
    void getparams(http_request message);
    void setparams(http_request message);


    // FSMW commands and transitions managment
    void addCommand(std::string s,CMDFunctor f);
        /**
       \brief Register a new state
       
       \param statename the name of the state
     */
    void addState(std::string statename);

    /**
       \brief register a transition
       \param cmd Transition name
       \param istate Initial state
       \param fstate Final state
       \param f PFunctor(see fsmTransition class)
     */
    void addTransition(std::string cmd,std::string istate,std::string fstate,CMDFunctor f);

    /**
       \brief Set the current state
       \param s the state name
     */
    void setState(std::string s);
    
    std::string state();///< Current state name
    void publishState();///< Unused

    /**
       \brief List all possible transitions
       \return JSON list of transitions
     */
    web::json::value transitionsList();
    
    /**
       \brief List all possible transitions for the current state
       \return JSON list of transitions
     */
    web::json::value allowList();

  private:
    std::map<std::string,CMDFunctor> _commands;
    std::vector<std::string> _states;
    std::string _state;
    std::map<std::string,std::vector<fsmTransition> > _transitions;

  protected:
    std::string _basePath;
    std::string _p_session;
    std::string _p_name;
    uint32_t _p_instance;
    web::json::value _params;
    
  };


