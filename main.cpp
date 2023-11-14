#include <boost/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/signals2/signal.hpp>

/*
https://stackoverflow.com/questions/5050588/how-in-boost-send-a-signal-in-a-thread-and-have-the-corresponding-slot-executed
*/

class IOService
{
public:
  IOService() : m_worker(boost::asio::make_work_guard(m_service)) {}
  ~IOService() {}

  // slot to receive signal
  void slotMessage(std::string msg)
  {
    m_service.post(boost::bind(&IOService::process, this, msg));
  }

  // start/close background thread
  bool start()
  {
    if (m_started)
      return true;
    m_started = true;

    // start reader thread
    m_thread = boost::thread(boost::bind(&IOService::loop, this));
    return m_started;
  }

  void loop()
  {
    m_service.run();
  }

  void close()
  {
    m_worker.reset();
    if (m_thread.joinable())
      m_thread.join();
    m_started = false;
  }

  // process
  void process(std::string msg)
  {
    printf("process %s\n", msg.c_str());
  }

private:
  bool m_started = false;
  boost::asio::io_service m_service;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_worker;
  boost::thread m_thread;
};

int main()
{
  // service instance
  IOService serv;
  serv.start();

  // signal to slot
  boost::signals2::signal<void(std::string)> signalMessage;
  signalMessage.connect(boost::bind( &IOService::slotMessage, boost::ref(serv)) );

  // send one signal
  signalMessage("abc");

  // wait and quit
  boost::this_thread::sleep(boost::chrono::seconds(2));
  serv.close();
}