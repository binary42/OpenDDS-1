// -*- C++ -*-
// ============================================================================
/**
 *  @file   publisher.cpp
 *
 *  $Id$
 *
 *
 */
// ============================================================================

#include "MessengerTypeSupportImpl.h"
#include "Writer.h"
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/PublisherImpl.h>
#include <dds/DCPS/transport/framework/TheTransportFactory.h>
#include <dds/DCPS/transport/tcp/TcpConfiguration.h>

#include "ace/streams.h"
#include "ace/Get_Opt.h"
#include "ace/OS_NS_sys_stat.h"

#include <string>
#include <fstream>

using namespace Messenger;

OpenDDS::DCPS::TransportIdType transport_impl_id = 1;
std::string publisher_trigger ("publisher_trigger");
std::string driver_trigger ("driver_trigger");

int
parse_args (int argc, ACE_TCHAR *argv[])
{
  ACE_Get_Opt get_opts (argc, argv, ACE_TEXT("t:i:e:"));
  int c;
  ACE_TCHAR *tmp;

  while ((c = get_opts ()) != -1)
  {
    switch (c)
    {
    case 'i':
      if ((tmp = get_opts.opt_arg ()) != 0) {
        publisher_trigger = ACE_TEXT_ALWAYS_CHAR(tmp);
      }
      break;
    case 'e':
      if ((tmp = get_opts.opt_arg ()) != 0) {
        driver_trigger = ACE_TEXT_ALWAYS_CHAR(tmp);
      }
      break;
    case 't':
      if (ACE_OS::strcmp (get_opts.opt_arg (), ACE_TEXT("udp")) == 0) {
        transport_impl_id = 2;
      }
      else if (ACE_OS::strcmp (get_opts.opt_arg (), ACE_TEXT("multicast")) == 0) {
        transport_impl_id = 3;
      }
      // test with DEFAULT_TCP_ID.
      else if (ACE_OS::strcmp (get_opts.opt_arg (), ACE_TEXT("default_tcp")) == 0) {
        transport_impl_id = OpenDDS::DCPS::DEFAULT_TCP_ID;
      }
      // test with DEFAULT_UDP_ID.
      else if (ACE_OS::strcmp (get_opts.opt_arg (), ACE_TEXT("default_udp")) == 0) {
        transport_impl_id = OpenDDS::DCPS::DEFAULT_UDP_ID;
      }
      else if (ACE_OS::strcmp (get_opts.opt_arg (), ACE_TEXT("default_multicast")) == 0) {
        transport_impl_id = OpenDDS::DCPS::DEFAULT_MULTICAST_ID;
      }
      break;
    case '?':
    default:
      ACE_ERROR_RETURN ((LM_ERROR,
        ACE_TEXT("usage:  %s -t <tcp/udp/default> n"),
        argv [0]),
        -1);
    }
  }
  // Indicates sucessful parsing of the command line
  return 0;
}

int ACE_TMAIN (int argc, ACE_TCHAR *argv[]) {
  try
    {
      DDS::DomainParticipantFactory_var dpf =
        TheParticipantFactoryWithArgs(argc, argv);
      DDS::DomainParticipant_var participant =
        dpf->create_participant(411,
                                PARTICIPANT_QOS_DEFAULT,
                                DDS::DomainParticipantListener::_nil(),
                                ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
      if (CORBA::is_nil (participant.in ())) {
        cerr << "create_participant failed." << endl;
        return 1;
      }

      if (parse_args (argc, argv) == -1) {
        return -1;
      }

      {
        // At this point we are connected to the Info Repo.
        // Trigger the driver
        std::ofstream ior_stream (driver_trigger.c_str());
        if (!ior_stream) {
          std::cerr << "Unable to open internal trigger file: "
                    << driver_trigger << std::endl;
          return -1;
        }
        ior_stream << "junk";
      }

      int max_wait_time = 30; //seconds
      int count = 0;
      while (true)
      {
        if (count > max_wait_time) {
          std::cerr << "Timed out waiting for external file: "
                    << publisher_trigger << std::endl;
          return -1;
        }

        // check for file
        ACE_stat my_stat;
        if (ACE_OS::stat (publisher_trigger.c_str(), &my_stat) == 0) {
          // found the trigger file.
          break;
        }

        ACE_OS::sleep (1);
      }

      MessageTypeSupportImpl* servant = new MessageTypeSupportImpl();
      OpenDDS::DCPS::LocalObject_var safe_servant = servant;

      if (DDS::RETCODE_OK != servant->register_type(participant.in (), "")) {
        cerr << "register_type failed." << endl;
        exit(1);
      }

      CORBA::String_var type_name = servant->get_type_name ();

      DDS::TopicQos topic_qos;
      participant->get_default_topic_qos(topic_qos);
      DDS::Topic_var topic =
        participant->create_topic ("Movie Discussion List",
                                   type_name.in (),
                                   topic_qos,
                                   DDS::TopicListener::_nil(),
                                   ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
      if (CORBA::is_nil (topic.in ())) {
        cerr << "create_topic failed." << endl;
        exit(1);
      }

      OpenDDS::DCPS::TransportImpl_rch tcp_impl =
        TheTransportFactory->create_transport_impl (transport_impl_id,
                                                    ::OpenDDS::DCPS::AUTO_CONFIG);

      DDS::Publisher_var pub =
        participant->create_publisher(PUBLISHER_QOS_DEFAULT,
                                      DDS::PublisherListener::_nil(),
                                      ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
      if (CORBA::is_nil (pub.in ())) {
        cerr << "create_publisher failed." << endl;
        exit(1);
      }

      // Attach the publisher to the transport.
      OpenDDS::DCPS::PublisherImpl* pub_impl =
        dynamic_cast< OpenDDS::DCPS::PublisherImpl*>(pub.in ());
      if (0 == pub_impl) {
        cerr << "Failed to obtain publisher servant" << endl;
        exit(1);
      }

      OpenDDS::DCPS::AttachStatus status = pub_impl->attach_transport(tcp_impl.in());
      if (status != OpenDDS::DCPS::ATTACH_OK) {
        std::string status_str;
        switch (status) {
        case OpenDDS::DCPS::ATTACH_BAD_TRANSPORT:
          status_str = "ATTACH_BAD_TRANSPORT";
          break;
        case OpenDDS::DCPS::ATTACH_ERROR:
          status_str = "ATTACH_ERROR";
          break;
        case OpenDDS::DCPS::ATTACH_INCOMPATIBLE_QOS:
          status_str = "ATTACH_INCOMPATIBLE_QOS";
          break;
        default:
          status_str = "Unknown Status";
          break;
        }
        cerr << "Failed to attach to the transport. Status == "
          << status_str.c_str() << endl;
        exit(1);
      }

      // Create the datawriter
      DDS::DataWriterQos dw_qos;
      pub->get_default_datawriter_qos (dw_qos);
      DDS::DataWriter_var dw =
        pub->create_datawriter(topic.in (),
                               dw_qos,
                               DDS::DataWriterListener::_nil(),
                               ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
      if (CORBA::is_nil (dw.in ())) {
        cerr << "create_datawriter failed." << endl;
        exit(1);
      }
      Writer* writer = new Writer(dw.in());

      writer->start ();
      while ( !writer->is_finished()) {
        ACE_Time_Value small_time(0,250000);
        ACE_OS::sleep (small_time);
      }

      // Cleanup
      writer->end ();
      delete writer;
      participant->delete_contained_entities();
      dpf->delete_participant(participant.in ());
      TheTransportFactory->release();
      TheServiceParticipant->shutdown ();
  }
  catch (CORBA::Exception& e)
    {
       cerr << "PUB: Exception caught in main.cpp:" << endl
         << e << endl;
      exit(1);
    }

  return 0;
}
