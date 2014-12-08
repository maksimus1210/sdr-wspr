#ifndef RECEIVER_HH
#define RECEIVER_HH

#include <QObject>
#include <QTimer>
#include <QStandardItemModel>
#include <QSettings>

#include "sources.hh"
#include "portaudio.hh"
#include "utils.hh"
#include "wspr2.hh"

class Receiver : public QObject
{
  Q_OBJECT

public:
  typedef enum {
    RTL_SOURCE, AUDIO_SOURCE, FILE_SOURCE
  } SourceType;

public:
  explicit Receiver(SourceType type, QObject *parent = 0);
  virtual ~Receiver();

  sdr::gui::SpectrumProvider *spectrum();

  void join();

  SourceType sourceType() const;
  void setSourceType(SourceType type);

  QWidget *createSourceControl();

  double frequency() const;
  void setFrequency(double F);

  double bfoFrequency() const;
  void setBfoFrequency(double F);

  bool audioAGCEnabled() const;
  void enableAudioAGC(bool enabled);

  bool monitorEnabled() const;
  void enableMonitor(bool enabled);

  QString locator() const;
  void setLocator(const QString &loc);

  QStandardItemModel *messages();

signals:
  void messagesReceived();

protected:
  void _onQueueStart();
  void _onQueueStop();
  void _onMessagesReceived();

protected slots:
  void _onStartRX();

protected:
  /** BFO frequency. */
  double _Fbfo;
  QString _qth;

  SourceType _sourceType;
  WSPRSource *_source;

  sdr::AGC<int16_t> _agc;
  sdr::WSPR2 _wspr;
  sdr::PortSink _audio;
  sdr::Queue &_queue;
  bool _monitor;

  QStandardItemModel *_messages;
  QTimer _timer;

  /** Persistent settings. */
  QSettings _settings;
};

#endif // RECEIVER_HH
