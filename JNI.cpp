// JNI Interface

// example input: "program -i dataset.slam -load build/lib/libkfusion-cpp-library.so"
std::vector<std::string> string_to_vector ( std::string str )
{
  using namespace std;
  stringstream ss(str);
  string item;
  vector<string> stringVector;
  while (getline(ss, item, ' '))
  {
    stringVector.push_back(item);
  }
  return stringVector;
}
char ** string_to_array ( std::string str ) {
 auto argvv = string_to_vector (str) ;
 char ** argv = (char **) malloc (argvv.size() * sizeof (char *));
 for (int i = 0 ; i < argvv.size(); i++) {
  argv[i] = (char*) malloc (argvv[i].length() * sizeof (char));
 } 
 return argv;
}

void clean_argv (char** argv , int argc) {

}
void start_slam ( std::string arguments )  
{ 
  //parse the arguements string as above to get argc, argv

  std::vector<std::string> argv = string_to_vector(arguments); 

  //todo: arguments in string character 

  int argc = argv.size();

  SLAMBenchConfiguration * config = new SLAMBenchConfiguration();

  config->GetParameterManager().ReadArgumentsOrQuit(argc, argv);

  config->InitAlgorithms();
}


void process_once ( std::string ) {
  SLAMBenchConfiguration::compute_loop_algorithm_once (config,NULL,NULL);
}


void SLAMBenchConfiguration::compute_loop_algorithm_once(SLAMBenchConfiguration* config, bool *remote_stay_on, SLAMBenchUI *ui) 
{

  assert(config->initialised_);

  for (auto lib : config->libs) 
  {
    auto trajectory = lib->GetOutputManager().GetMainOutput(slambench::values::VT_POSE);
    if(trajectory == nullptr) 
    {
      std::cerr << "Algo does not provide a main pose output" << std::endl;
      exit(1);
    }
  }

  // ********* [[ COMPUTE LOOP ONCE ]] *********
  bool ongoing = false;

  // ********* [[ LOAD A NEW FRAME ]] *********

  if(config->input_stream_ == nullptr) 
  {
    std::cerr << "No input loaded." << std::endl;
    return;
  }
  
  slambench::io::SLAMFrame * next_frame = config->input_stream_->GetNextFrame();

  if (next_frame == nullptr) 
  {
    std::cerr << "Last frame processed." << std::endl;
    return;
  }
  
  // ********* [[ NEW FRAME PROCESSED BY ALGO ]] *********

  for (auto lib : config->libs) 
  {
    // ********* [[ SEND THE FRAME ]] *********
    ongoing=not lib->c_sb_update_frame(lib,next_frame);
    
    // This algorithm hasn't received enough frames yet.
    if(ongoing) 
    {
      continue;
    }

    // ********* [[ PROCESS ALGO START ]] *********
    lib->GetMetricManager().BeginFrame();
    
    if (not lib->c_sb_process_once (lib)) 
    {
      std::cerr <<"Error after lib->c_sb_process_once." << std::endl;
      exit(1);
    }
    
    slambench::TimeStamp ts = next_frame->Timestamp;
    if(!lib->c_sb_update_outputs(lib, &ts)) 
    {
      std::cerr << "Failed to get outputs" << std::endl;
      exit(1);
    }
    
    lib->GetMetricManager().EndFrame();
  }

  // ********* [[ FINALIZE ]] *********

  next_frame->FreeData();

  if(!ongoing) 
  {
    config->FireEndOfFrame();
    if (ui) ui->stepFrame();
  }
}

void get_visual () {

  for(auto lib : config->GetLoadedLibs()) 
  {
    for (auto output : lib->GetOutputmanager() ) 
    {
      // After this process once I've got twnety two outputs
      if (output.second->IsActive() and (output.second->GetType() == slambench::values::VT_FRAME) and outputs_background_.at(output.second)->Get()) 
      {
        slambench::values::FrameValue* fv = output.second->GetValues().rbegin()->second;
        fv->Width
        fv->Height
        fv->Data
      }
    }
  } 
}

release () {
  delete config;
}

