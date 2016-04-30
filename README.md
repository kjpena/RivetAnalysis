RivetAnalysis
=============

## Installation

Install CMSSW and download this package and the generator configuration files
```
cmsrel CMSSW_7_4_0
cd CMSSW_7_4_0/src
cmsenv
git cms-addpkg GeneratorInterface/RivetInterface 
git clone git@github.com:pfs/RivetAnalysis.git UserCode/RivetAnalysis
scram b -j 9
```
If you need to use yoda scripts, you can use the following extra variables
```
yodaver=1.3.0-cms2
export PATH=${PATH}:/cvmfs/cms.cern.ch/$SCRAM_ARCH/external/yoda/$yodaver/bin
```

## Running RIVET plugins

There are several plugins implemented under src/. 
   * CMS_TOP_13_007.cc - TOP-13-007 UE measurement in the dileptons channel  
   * CMS_TOP_Dileptons.cc - TOP-16-002 mass from lepton kinematics
   * CMS_TOP_Radius.cc - composite top analysis
A wrapper script to call the RIVET plugin while generating events on the fly is provided 
under scripts/. The script takes a configuration file used to generate the events and 
customizes the run according to the plugin to be used.
An example is as follows
```
python scripts/submitRivetRun.py  -n -1 -q local 
       -o results_Rt_0_kappa_0 
       -i /store/cmst3/group/top/summer2016/composite_tt01j_lhe/Rt_0_kappa_0.lhe 
       -c UserCode/RivetAnalysis/python/Hadronizer_MgmMatchTuneCUEP8S1CTEQ6L1_13TeV_madgraph_pythia8_Tauola_cff
       -r UserCode/RivetAnalysis/rivet_customise.customiseTOPRadius
```
If local is changed to a batch queue (e.g. 8nh) the job is submited to lxbatch for execution
The output of the job can be plotted using the standard tools

```
rivet-mkhtml -s --mc-errs -o ~/public/html/TopRadius --times \
	     results/Rt_0_kappa_0/out_1.yoda:'SM' \
	     results/Rt_5e_6_kappa_0/out_1.yoda:'R_{t}=5e-6' \
```
An html page will be generated and can be opened from a browser @
```
http://cmsdoc.cern.ch/~`whoami`/TopRadius/index.html
```

