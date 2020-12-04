 /* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * NIST-developed software is provided by NIST as a public
 * service. You may use, copy and distribute copies of the software in
 * any medium, provided that you keep intact this entire notice. You
 * may improve, modify and create derivative works of the software or
 * any portion of the software, and you may copy and distribute such
 * modifications or works. Modified works should carry a notice
 * stating that you changed the software and should note the date and
 * nature of any such change. Please explicitly acknowledge the
 * National Institute of Standards and Technology as the source of the
 * software.
 *
 * NIST-developed software is expressly provided "AS IS." NIST MAKES
 * NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY
 * OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
 * WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED
 * OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT
 * WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE
 * SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE
 * CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
 *
 * You are solely responsible for determining the appropriateness of
 * using and distributing the software and you assume all risks
 * associated with its use, including but not limited to the risks and
 * costs of program errors, compliance with applicable laws, damage to
 * or loss of data, programs or equipment, and the unavailability or
 * interruption of operation. This software is not intended to be used
 * in any situation where a failure could cause risk of injury or
 * damage to property. The software developed by NIST employees is not
 * subject to copyright protection within the United States.
 */


#include "ns3/lte-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store.h"
#include <cfloat>
#include <sstream>
#include <math.h>

using namespace ns3;

// This trace will log packet transmissions and receptions from the application
// layer.  The parameter 'localAddrs' is passed to this trace in case the
// address passed by the trace is not set (i.e., is '0.0.0.0' or '::').  The
// trace writes to a file stream provided by the first argument; by default,
// this trace file is 'UePacketTrace.tr'
//
//

double distance(double a[], double b[]){
       double d;
       d= pow(a[0]-b[0],2)+pow(a[1]-b[1],2)+pow(a[2]-b[2],2);
       return d;
 }


void
UePacketTrace (Ptr<OutputStreamWrapper> stream, const Address &localAddrs, std::string context, Ptr<const Packet> p, const Address &srcAddrs, const Address &dstAddrs)
{
  std::ostringstream oss;
  *stream->GetStream () << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << context << "\t" << p->GetSize () << "\t";
  if (InetSocketAddress::IsMatchingType (srcAddrs))
    {
      oss << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 ();
      if (!oss.str ().compare ("0.0.0.0")) //srcAddrs not set
        {
          *stream->GetStream () << Ipv4Address::ConvertFrom (localAddrs) << ":" << InetSocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
        }
      else
        {
          oss.str ("");
          oss << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 ();
          if (!oss.str ().compare ("0.0.0.0")) //dstAddrs not set
            {
              *stream->GetStream () << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" <<  Ipv4Address::ConvertFrom (localAddrs) << ":" << InetSocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
          else
            {
              *stream->GetStream () << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
        }
    }
  else if (Inet6SocketAddress::IsMatchingType (srcAddrs))
    {
      oss << Inet6SocketAddress::ConvertFrom (srcAddrs).GetIpv6 ();
      if (!oss.str ().compare ("::")) //srcAddrs not set
        {
          *stream->GetStream () << Ipv6Address::ConvertFrom (localAddrs) << ":" << Inet6SocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
        }
      else
        {
          oss.str ("");
          oss << Inet6SocketAddress::ConvertFrom (dstAddrs).GetIpv6 ();
          if (!oss.str ().compare ("::")) //dstAddrs not set
            {
              *stream->GetStream () << Inet6SocketAddress::ConvertFrom (srcAddrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << Ipv6Address::ConvertFrom (localAddrs) << ":" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
          else
            {
              *stream->GetStream () << Inet6SocketAddress::ConvertFrom (srcAddrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
        }
    }
  else
    {
      *stream->GetStream () << "Unknown address type!" << std::endl;
    }
}

/*
 * The topology is the following:
 *
 *    UE2 (-10.0, 0.0, 1.5).....(10 m).....eNB (0.0, 0.0, 30.0).....(10 m).....UE1 (10.0, 0.0, 1.5)
 *
 * Please refer to the Sidelink section of the LTE user documentation for more details.
 *
 */

NS_LOG_COMPONENT_DEFINE ("LteSlInCovrgCommMode1");

int main (int argc, char *argv[])
{
  Time simTime = Seconds (6);
  bool enableNsLogs = false;
  bool useIPv6 = false;

  CommandLine cmd;
  cmd.AddValue ("simTime", "Total duration of the simulation", simTime);
  cmd.AddValue ("enableNsLogs", "Enable ns-3 logging (debug builds)", enableNsLogs);
  cmd.AddValue ("useIPv6", "Use IPv6 instead of IPv4", useIPv6);
  cmd.Parse (argc, argv);

  // Configure the scheduler
  Config::SetDefault ("ns3::RrSlFfMacScheduler::Itrp", UintegerValue (0));
  //The number of RBs allocated per UE for Sidelink
  Config::SetDefault ("ns3::RrSlFfMacScheduler::SlGrantSize", UintegerValue (5));

  //Set the frequency

  Config::SetDefault ("ns3::LteEnbNetDevice::DlEarfcn", UintegerValue (100));
  Config::SetDefault ("ns3::LteUeNetDevice::DlEarfcn", UintegerValue (100));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlEarfcn", UintegerValue (18100));
  Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue (50));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", UintegerValue (50));


  // Set error models
  Config::SetDefault ("ns3::LteSpectrumPhy::SlCtrlErrorModelEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteSpectrumPhy::SlDataErrorModelEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteSpectrumPhy::DropRbOnCollisionEnabled", BooleanValue (false));

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();
  // parse again so we can override input file default values via command line
  cmd.Parse (argc, argv);

  if (enableNsLogs)
    {
      LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_ALL);

      LogComponentEnable ("LteUeRrc", logLevel);
      LogComponentEnable ("LteUeMac", logLevel);
      LogComponentEnable ("LteSpectrumPhy", logLevel);
      LogComponentEnable ("LteUePhy", logLevel);
      LogComponentEnable ("LteEnbPhy", logLevel);
    }

  //Set the UEs power in dBm
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (23.0));
  //Set the eNBs power in dBm
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (30.0));

  //Sidelink bearers activation time
  Time slBearersActivationTime = Seconds (2.0);

  //Create the helpers
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  //Create and set the EPC helper
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  ////Create Sidelink helper and set lteHelper
  Ptr<LteSidelinkHelper> proseHelper = CreateObject<LteSidelinkHelper> ();
  proseHelper->SetLteHelper (lteHelper);

  //Set pathloss model
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::Cost231PropagationLossModel"));

  //Enable Sidelink
  lteHelper->SetAttribute ("UseSidelink", BooleanValue (true));

  //Sidelink Round Robin scheduler
  lteHelper->SetSchedulerType ("ns3::RrSlFfMacScheduler");

  //Create nodes (eNb + UEs)
  NodeContainer enbNode;
  enbNode.Create (1);
  NS_LOG_INFO ("eNb node id = [" << enbNode.Get (0)->GetId () << "]");
  NodeContainer ueNodes;
  ueNodes.Create (10);

  NodeContainer cuNodes;
  cuNodes.Create(2);

  NS_LOG_INFO ("UE 1 node id = [" << ueNodes.Get (0)->GetId () << "]");
  NS_LOG_INFO ("UE 2 node id = [" << ueNodes.Get (1)->GetId () << "]");

  //Position of the nodes
  Ptr<ListPositionAllocator> positionAllocEnb = CreateObject<ListPositionAllocator> ();
  positionAllocEnb->Add (Vector (0.0, 0.0, 30.0));
  
  Ptr<ListPositionAllocator> positionAllocUe1 = CreateObject<ListPositionAllocator> ();
  positionAllocUe1->Add (Vector (10.0, 0.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocUe2 = CreateObject<ListPositionAllocator> ();
  positionAllocUe2->Add (Vector (-10.0, 0.0, 0.0));
   Ptr<ListPositionAllocator> positionAllocUe3 = CreateObject<ListPositionAllocator> ();
     positionAllocUe3->Add (Vector (40.0, 0.0, 0.0));
       Ptr<ListPositionAllocator> positionAllocUe4 = CreateObject<ListPositionAllocator> ();
         positionAllocUe4->Add (Vector (-25.0, 0.0, 0.0));
	 
	 Ptr<ListPositionAllocator> positionAllocUe5 = CreateObject<ListPositionAllocator> ();
	   positionAllocUe5->Add (Vector (-15.0, 0.0, 0.0));
	     Ptr<ListPositionAllocator> positionAllocUe6 = CreateObject<ListPositionAllocator> ();
	       positionAllocUe6->Add (Vector (-35.0, 0.0, 0.0));



  Ptr<ListPositionAllocator> positionAllocUe7 = CreateObject<ListPositionAllocator> ();
    positionAllocUe7->Add (Vector (35.0, 5.0, 0.0));
      Ptr<ListPositionAllocator> positionAllocUe8 = CreateObject<ListPositionAllocator> ();
        positionAllocUe8->Add (Vector (-45.0, 5.0, 0.0));
	   Ptr<ListPositionAllocator> positionAllocUe9 = CreateObject<ListPositionAllocator> ();
	        positionAllocUe9->Add (Vector (-35.0, 10.0, 0.0));
		       Ptr<ListPositionAllocator> positionAllocUe10 = CreateObject<ListPositionAllocator> ();
		                positionAllocUe10->Add (Vector (-45.0, 10.0, 0.0));


  //Install mobility
  MobilityHelper mobilityeNodeB;
  mobilityeNodeB.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityeNodeB.SetPositionAllocator (positionAllocEnb);
  mobilityeNodeB.Install (enbNode);

  MobilityHelper mobilityUe1;
  mobilityUe1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityUe1.SetPositionAllocator (positionAllocUe1);
  mobilityUe1.Install (ueNodes.Get (0));

  MobilityHelper mobilityUe2;
  mobilityUe2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityUe2.SetPositionAllocator (positionAllocUe2);
  mobilityUe2.Install (ueNodes.Get (1));

  MobilityHelper mobilityUe3;
    mobilityUe3.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobilityUe3.SetPositionAllocator (positionAllocUe3);
        mobilityUe3.Install (ueNodes.Get (2));

	  MobilityHelper mobilityUe4;
	    mobilityUe4.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	      mobilityUe4.SetPositionAllocator (positionAllocUe4);
	        mobilityUe4.Install (ueNodes.Get (3));

  MobilityHelper mobilityUe5;
    mobilityUe5.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobilityUe5.SetPositionAllocator (positionAllocUe5);
        mobilityUe5.Install (ueNodes.Get (4));

	  MobilityHelper mobilityUe6;
	    mobilityUe6.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	      mobilityUe6.SetPositionAllocator (positionAllocUe6);
	        mobilityUe6.Install (ueNodes.Get (5));

MobilityHelper mobilityUe7;
  mobilityUe7.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobilityUe7.SetPositionAllocator (positionAllocUe7);
      mobilityUe7.Install (ueNodes.Get (6));

        MobilityHelper mobilityUe8;
	  mobilityUe8.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	    mobilityUe8.SetPositionAllocator (positionAllocUe8);
	      mobilityUe8.Install (ueNodes.Get (7));

MobilityHelper mobilityUe9;
  mobilityUe9.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobilityUe9.SetPositionAllocator (positionAllocUe9);
            mobilityUe9.Install (ueNodes.Get (8));

	            MobilityHelper mobilityUe10;
		              mobilityUe10.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
			                  mobilityUe10.SetPositionAllocator (positionAllocUe10);
					                mobilityUe10.Install (ueNodes.Get (9));


  uint32_t n_CellularUsers=2;
  uint32_t n_d2dPairs=(ueNodes.GetN()-n_CellularUsers)/2;
  std::cout<<"Number of Cellular users: "<<n_CellularUsers<<"\nNumber of D2D pairs: "<<n_d2dPairs<<"\n";

  ////////////////////////
  
  ///////////////////////


  double CU_coordinates[n_CellularUsers+10][3];
  NS_LOG_INFO(CU_coordinates);
  double cu[n_CellularUsers*3];
  NS_LOG_INFO(cu);
  for (uint32_t i=0;i<n_CellularUsers;i++){
  	Ptr<MobilityModel> ueNode = ueNodes.Get (i)->GetObject<MobilityModel>();
	double x = ueNode->GetPosition().x;
	double y = ueNode->GetPosition().y;
	double z = ueNode->GetPosition().z;
	CU_coordinates[i][0]=x;
	CU_coordinates[i][1]=y;
	CU_coordinates[i][2]=z;
	cu[i*3+0]=x;
	cu[i*3+1]=y;
	cu[i*3+2]=z;	
	std::cout<<"Cellular User: ("<< x <<","<< y <<","<< z <<")\n";
	//std::cout<<CU_coordinates[i][0]<<" "<<CU_coordinates[i][1]<<" "<<CU_coordinates[i][2]<<"\n";
  }
//  std::cout<<CU_coordinates[0][0]<<"\n"<<CU_coordinates[0][1]<<"\n"<<CU_coordinates[0][2]<<"\n";
  //std::cout<<CU_coordinates[0][0]<<" "<<CU_coordinates[0][1]<<" "<<CU_coordinates[0][2]<<"\n";
  
  //////////////////////////////////////////////
  double d2d_coordinates[n_d2dPairs][3];	
  NS_LOG_INFO(d2d_coordinates); 
  double du[n_d2dPairs*3];
  NS_LOG_INFO(du);
  //std::cout<<CU_coordinates[0][0]<<" "<<CU_coordinates[0][1]<<" "<<CU_coordinates[0][2]<<"\n"; 
  
  int j=0;
  for( uint32_t i=n_CellularUsers ; i<ueNodes.GetN() ; i=i+2 ) {

	//std::cout<<CU_coordinates[0][0]<<" "<<CU_coordinates[0][1]<<" "<<CU_coordinates[0][2]<<"\n";
  
	Ptr<MobilityModel> ueNode_current = ueNodes.Get (i)->GetObject<MobilityModel>();
	Ptr<MobilityModel> ueNode_next = ueNodes.Get (i+1)->GetObject<MobilityModel>();
	double X1 = ueNode_current->GetPosition().x;
	double Y1 = ueNode_current->GetPosition().y;
	double Z1 = ueNode_current->GetPosition().z;
	double X2 = ueNode_next->GetPosition().x;
	double Y2 = ueNode_next->GetPosition().y;
	double Z2 = ueNode_next->GetPosition().z;
	double X = (X1+X2)/2;
	double Y = (Y1+Y2)/2;
	double Z = (Z1+Z2)/2;
	//std::cout<<CU_coordinates[0][0]<<" "<<CU_coordinates[0][1]<<" "<<CU_coordinates[0][2]<<"\n";
	d2d_coordinates[int(i)-int(n_CellularUsers)][0]=X;
	d2d_coordinates[int(i)-int(n_CellularUsers)][1]=Y;
	d2d_coordinates[int(i)-int(n_CellularUsers)][2]=Z;
	du[j]=X;j++;
	du[j]=Y;j++;
	du[j]=Z;j++;
	//std::cout<<CU_coordinates[0][0]<<" "<<CU_coordinates[0][1]<<" "<<CU_coordinates[0][2]<<"\n";
	std::cout<<"D2D User at: ("<< X1 <<","<< Y1 <<","<< Z1 <<")"<<"("<< X2 <<","<< Y2 <<","<< Z2 <<")"<<"\tAverage: ("<< X <<","<< Y <<","<< Z <<")"<<"\n";
	//std::cout<<CU_coordinates[0][0]<<" "<<CU_coordinates[0][1]<<" "<<CU_coordinates[0][2]<<"\n";
  }
  /////////////////////////////////////////////
//  CU_coordinates[0][0]=10;
//  d2d_coordinates[1][0]=-20;
//  d2d_coordinates[1][2]=0;
  
  for(int i=0;i<int(n_d2dPairs*3);i++){
  	//std::cout<<"\t\t"<<du[i]<<"\n";
  }

  //std::cout<<CU_coordinates[0][0]<<" "<<CU_coordinates[0][1]<<" "<<CU_coordinates[0][2]<<"\n";
  for(uint32_t i=0;i<2;i++){
	for(int j=0;j<3;j++){
		//std::cout<<"\t"<<CU_coordinates[i][j]<<"\n";
	}
  }
  std::cout<<"\n";
  for(uint32_t i=0;i<2;i++){
	          for(uint32_t j=0;j<3;j++){
			    //              std::cout<<"\t"<<d2d_coordinates[i][j]<<"\n";
					          }
		    }
  
  int result[n_d2dPairs];
  NS_LOG_INFO(result);  
  for (uint32_t i=0; i<n_d2dPairs; i++){
  	int r=0;
	NS_LOG_INFO(r);
	
	double b[]={du[3*i+0],du[3*i+1],du[3*i+2]};
	double dist=distance( CU_coordinates[0] , b );
	double minimum=distance( CU_coordinates[0] , b );
	for (uint32_t j=0;j<n_CellularUsers;j++){
		dist=distance( CU_coordinates[j] , b );
		//std::cout<<"Distance: "<<dist<<"\n";
		if(dist<minimum){
		minimum=dist;
		r=j;
		//std::cout<<"changed\n";
		}
	}
	result[i]=r;
	//std::cout<<"\t\t"<<result[i]<<"\n";
  }

  std::cout<<"Number of shards created:  "<<n_CellularUsers<<"\n";
  for(int i=0;i<int(n_CellularUsers);i++){
  	std::cout<<"SHARD "<<i+1<<":\n";
	std::cout<<"Celluar User present at location: "<<"("<<CU_coordinates[i][0] <<","<<CU_coordinates[i][1] <<","<<CU_coordinates[i][2] <<")\n";
	for(int j=0;j<int(n_d2dPairs);j++){
		if(result[j]==i){
			std::cout<<"D2D pair present at location: "<<"("<<du[3*j+0] <<","<<du[3*j+1] <<","<<du[3*j+2] <<")\n";
		}
	}
  std::cout<<"\n";
  }
  

  //Install LTE devices to the nodes and fix the random number stream
  int64_t randomStream = 1;
  NetDeviceContainer enbDevs = lteHelper->InstallEnbDevice (enbNode);
  randomStream += lteHelper->AssignStreams (enbDevs, randomStream);
  NetDeviceContainer ueDevs = lteHelper->InstallUeDevice (ueNodes);
  randomStream += lteHelper->AssignStreams (ueDevs, randomStream);

  //Configure Sidelink Pool
  Ptr<LteSlEnbRrc> enbSidelinkConfiguration = CreateObject<LteSlEnbRrc> ();
  enbSidelinkConfiguration->SetSlEnabled (true);

  //Preconfigure pool for the group
  LteRrcSap::SlCommTxResourcesSetup pool;

  pool.setup = LteRrcSap::SlCommTxResourcesSetup::SCHEDULED;
  //BSR timers
  pool.scheduled.macMainConfig.periodicBsrTimer.period = LteRrcSap::PeriodicBsrTimer::sf16;
  pool.scheduled.macMainConfig.retxBsrTimer.period = LteRrcSap::RetxBsrTimer::sf640;
  //MCS
  pool.scheduled.haveMcs = true;
  pool.scheduled.mcs = 16;
  //resource pool
  LteSlResourcePoolFactory pfactory;
  pfactory.SetHaveUeSelectedResourceConfig (false); //since we want eNB to schedule

  //Control
  pfactory.SetControlPeriod ("sf40");
  pfactory.SetControlBitmap (0x00000000FF); //8 subframes for PSCCH
  pfactory.SetControlOffset (0);
  pfactory.SetControlPrbNum (22);
  pfactory.SetControlPrbStart (0);
  pfactory.SetControlPrbEnd (49);

  //Data: The ns3::RrSlFfMacScheduler is responsible to handle the parameters


  pool.scheduled.commTxConfig = pfactory.CreatePool ();

  uint32_t groupL2Address = 255;

  enbSidelinkConfiguration->AddPreconfiguredDedicatedPool (groupL2Address, pool);
  lteHelper->InstallSidelinkConfiguration (enbDevs, enbSidelinkConfiguration);

  //pre-configuration for the UEs
  Ptr<LteSlUeRrc> ueSidelinkConfiguration = CreateObject<LteSlUeRrc> ();
  ueSidelinkConfiguration->SetSlEnabled (true);
  LteRrcSap::SlPreconfiguration preconfiguration;
  ueSidelinkConfiguration->SetSlPreconfiguration (preconfiguration);
  lteHelper->InstallSidelinkConfiguration (ueDevs, ueSidelinkConfiguration);

  InternetStackHelper internet;
  internet.Install (ueNodes);
  Ipv4Address groupAddress4 ("225.0.0.0");     //use multicast address as destination
  Ipv6Address groupAddress6 ("ff0e::1");     //use multicast address as destination
  Address remoteAddress, remote0, remote1, remote2,remote3, remote[4];
  Address localAddress, local0,local1,local2,local3,local[4];
  Ptr<LteSlTft> tft;
  if (!useIPv6)
    {
      //Install the IP stack on the UEs and assign IP address
      Ipv4InterfaceContainer ueIpIface;
      ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));

      // set the default gateway for the UE
      Ipv4StaticRoutingHelper ipv4RoutingHelper;
      for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = ueNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }
      remote[0] = InetSocketAddress (groupAddress4, 8000);
      local[0] = InetSocketAddress (Ipv4Address::GetAny (), 8000);

      remote[1] = InetSocketAddress (groupAddress4, 8001);
      local[1] = InetSocketAddress (Ipv4Address::GetAny (), 8001);

      remote[2] = InetSocketAddress (groupAddress4, 8002);
      local[2] = InetSocketAddress (Ipv4Address::GetAny (), 8002);

      remote[3] = InetSocketAddress (groupAddress4, 8003);
      local[3] = InetSocketAddress (Ipv4Address::GetAny (), 8003);

      tft = Create<LteSlTft> (LteSlTft::BIDIRECTIONAL, groupAddress4, groupL2Address);
    }
  else
    {
      Ipv6InterfaceContainer ueIpIface;
      ueIpIface = epcHelper->AssignUeIpv6Address (NetDeviceContainer (ueDevs));

      // set the default gateway for the UE
      Ipv6StaticRoutingHelper Ipv6RoutingHelper;
      for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = ueNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv6StaticRouting> ueStaticRouting = Ipv6RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv6> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress6 (), 1);
        }
      remoteAddress = Inet6SocketAddress (groupAddress6, 8000);
      localAddress = Inet6SocketAddress (Ipv6Address::GetAny (), 8000);
      tft = Create<LteSlTft> (LteSlTft::BIDIRECTIONAL, groupAddress6, groupL2Address);
    }

  //Attach each UE to the best available eNB
  lteHelper->Attach (ueDevs);

  ///*** Configure applications ***///
  //
  //std:cout<<"Number: "<<ueNodes.GetN();	
//NS_LOG_INFO ("Number of UE:" << ueNodes.GetN ());
//std::cout<<"Number of UE:" << ueNodes.GetN ();

  int x[10];
  for(int i=0;i<10;i++){
	x[i]=0;
  }
  
  //Set Application in the UEs
  OnOffHelper sidelinkClient0 ("ns3::UdpSocketFactory", remote[0]);
  sidelinkClient0.SetConstantRate (DataRate ("16kb/s"), 200);

  ApplicationContainer clientApps[4];
  clientApps[0] = sidelinkClient0.Install (ueNodes.Get (2));
  //onoff application will send the first packet at :
  //(2.9 (App Start Time) + (1600 (Pkt size in bits) / 16000 (Data rate)) = 3.0 sec
  int t=2+x[result[0]]++;
  //std::cout<<"\t"<<t<<"\n";
  clientApps[0].Start (Seconds (t));
  clientApps[0].Stop (Seconds (10));

  ApplicationContainer serverApps[4];
  PacketSinkHelper sidelinkSink0 ("ns3::UdpSocketFactory", local[0]);
  serverApps[0] = sidelinkSink0.Install (ueNodes.Get (3));
  serverApps[0].Start (Seconds (t));
  serverApps[0].Stop (Seconds (10.0));

  /////////////////////////////
  OnOffHelper sidelinkClient1 ("ns3::UdpSocketFactory", remote[1]);
  sidelinkClient1.SetConstantRate (DataRate ("16kb/s"), 200);
  clientApps[1] = sidelinkClient1.Install (ueNodes.Get (4));
  t=2+x[result[1]]++;
  //std::cout<<"\t"<<t<<"\n";
  clientApps[1].Start (Seconds (t));
  clientApps[1].Stop (Seconds (10));

  //ApplicationContainer serverApps2;
  PacketSinkHelper sidelinkSink1 ("ns3::UdpSocketFactory", local[1]);
  serverApps[1] = sidelinkSink1.Install (ueNodes.Get (5));
  serverApps[1].Start (Seconds (t));
  serverApps[1].Stop (Seconds (10.0));

  //////////////////////////////
  
  OnOffHelper sidelinkClient2 ("ns3::UdpSocketFactory", remote[2]);
  sidelinkClient2.SetConstantRate (DataRate ("16kb/s"), 200);
  clientApps[2] = sidelinkClient2.Install (ueNodes.Get (6));
  t=2+x[result[2]]++;
  //std::cout<<"\t"<<t<<"\n";
  clientApps[2].Start (Seconds (t));
  clientApps[2].Stop (Seconds (10));

  //ApplicationContainer serverApps2;
  PacketSinkHelper sidelinkSink2 ("ns3::UdpSocketFactory", local[2]);
  serverApps[2] = sidelinkSink2.Install (ueNodes.Get (7));
  serverApps[2].Start (Seconds (t));
  serverApps[2].Stop (Seconds (10.0));

  /////////////////////////////

  OnOffHelper sidelinkClient3 ("ns3::UdpSocketFactory", remote[3]);
  sidelinkClient3.SetConstantRate (DataRate ("16kb/s"), 200);
  clientApps[3] = sidelinkClient3.Install (ueNodes.Get (8));
  t=2+x[result[3]]++;
  //std::cout<<"\t"<<t<<"\n";
  clientApps[3].Start (Seconds (t));
  clientApps[3].Stop (Seconds (10));

  //ApplicationContainer serverApps3;
  PacketSinkHelper sidelinkSink3 ("ns3::UdpSocketFactory", local[3]);
  serverApps[3] = sidelinkSink3.Install (ueNodes.Get (9));
  serverApps[3].Start (Seconds (t));
  serverApps[3].Stop (Seconds (10.0));

  /////////////////////////////

  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, ueDevs, tft);
  ///*** End of application configuration ***///

  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("UePacketTrace.tr");

  //Trace file table header
  *stream->GetStream () << "time(sec)\ttx/rx\tNodeID\tIMSI\tPktSize(bytes)\tIP[src]\tIP[dst]" << std::endl;

  std::ostringstream oss;

  if (!useIPv6)
    {
      // Set Tx traces
      for (uint16_t ac = 0; ac < clientApps[0].GetN (); ac++)
        {
          Ipv4Address localAddrs =  clientApps[0].Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          std::cout << "Tx address: " << localAddrs << std::endl;
          oss << "tx\t" << ueNodes.Get (2)->GetId () << "\t" << ueNodes.Get (2)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          clientApps[0].Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
	  
        }
      for (uint16_t ac = 0; ac < clientApps[1].GetN (); ac++){
	   Ipv4Address localAddrs =  clientApps[1].Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
           std::cout << "Tx address: " << localAddrs << std::endl;
           oss << "tx\t" << ueNodes.Get (4)->GetId () << "\t" << ueNodes.Get (4)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
           clientApps[1].Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
           oss.str ("");
           }
	
      for (uint16_t ac = 0; ac < clientApps[2].GetN (); ac++){
	   Ipv4Address localAddrs =  clientApps[2].Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
           std::cout << "Tx address: " << localAddrs << std::endl;
           oss << "tx\t" << ueNodes.Get (6)->GetId () << "\t" << ueNodes.Get (6)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
           clientApps[2].Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
           oss.str ("");
           }


      for (uint16_t ac = 0; ac < clientApps[3].GetN (); ac++){
	   Ipv4Address localAddrs =  clientApps[3].Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
           std::cout << "Tx address: " << localAddrs << std::endl;
           oss << "tx\t" << ueNodes.Get (8)->GetId () << "\t" << ueNodes.Get (8)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
           clientApps[3].Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
           oss.str ("");
           }
      

      // Set Rx traces
      for (uint16_t ac = 0; ac < serverApps[0].GetN (); ac++)
        {
          Ipv4Address localAddrs =  serverApps[0].Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          std::cout << "Rx address: " << localAddrs << std::endl;
          oss << "rx\t" << ueNodes.Get (3)->GetId () << "\t" << ueNodes.Get (3)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          serverApps[0].Get (ac)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }


      for (uint16_t ac = 0; ac < serverApps[1].GetN (); ac++){
      	  Ipv4Address localAddrs =  serverApps[1].Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
	   std::cout << "Rx address: " << localAddrs << std::endl; 
	  oss << "rx\t" << ueNodes.Get (5)->GetId () << "\t" << ueNodes.Get (5)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
	   serverApps[1].Get (ac)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
	   oss.str ("");
      }

      for (uint16_t ac = 0; ac < serverApps[2].GetN (); ac++){
      	  Ipv4Address localAddrs =  serverApps[2].Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
	   std::cout << "Rx address: " << localAddrs << std::endl; 
	  oss << "rx\t" << ueNodes.Get (7)->GetId () << "\t" << ueNodes.Get (7)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
	   serverApps[2].Get (ac)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
	   oss.str ("");
      }

      for (uint16_t ac = 0; ac < serverApps[3].GetN (); ac++){
      	  Ipv4Address localAddrs =  serverApps[3].Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
	   std::cout << "Rx address: " << localAddrs << std::endl; 
	  oss << "rx\t" << ueNodes.Get (9)->GetId () << "\t" << ueNodes.Get (9)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
	   serverApps[3].Get (ac)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
	   oss.str ("");
      }

    }
  else
    {
      // Set Tx traces
      for (uint16_t ac = 0; ac < clientApps[0].GetN (); ac++)
        {
          clientApps[0].Get (ac)->GetNode ()->GetObject<Ipv6L3Protocol> ()->AddMulticastAddress (groupAddress6);
          Ipv6Address localAddrs =  clientApps[0].Get (ac)->GetNode ()->GetObject<Ipv6L3Protocol> ()->GetAddress (1,1).GetAddress ();
          std::cout << "Tx address: " << localAddrs << std::endl;
          oss << "tx\t" << ueNodes.Get (0)->GetId () << "\t" << ueNodes.Get (0)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          clientApps[0].Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }

      // Set Rx traces
      for (uint16_t ac = 0; ac < serverApps[0].GetN (); ac++)
        {
          serverApps[0].Get (ac)->GetNode ()->GetObject<Ipv6L3Protocol> ()->AddMulticastAddress (groupAddress6);
          Ipv6Address localAddrs =  serverApps[0].Get (ac)->GetNode ()->GetObject<Ipv6L3Protocol> ()->GetAddress (1,1).GetAddress ();
          std::cout << "Rx address: " << localAddrs << std::endl;
          oss << "rx\t" << ueNodes.Get (1)->GetId () << "\t" << ueNodes.Get (1)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          serverApps[0].Get (ac)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }
    }

  NS_LOG_INFO ("Enabling Sidelink traces...");
  lteHelper->EnableSidelinkTraces ();

  NS_LOG_INFO ("Starting simulation...");

  Simulator::Stop (simTime);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;

}

