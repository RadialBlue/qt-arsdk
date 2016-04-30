#ifndef ARCONTROLCONNECTION_H
#define ARCONTROLCONNECTION_H

#include <QObject>
#include <QVariantMap>

class ARController;
class ARControlFrame;

class ARCommandInfo;
class ARCommandListener;

class ARControlConnection : public QObject
{
    Q_OBJECT

public:
    typedef enum {
        NotInitialized = 0,
        Acknowledge,
        Data,
        LowLatencyData,
        AcknowledgeData
    } FrameType;

    explicit ARControlConnection(ARController *controller);
            ~ARControlConnection();

    bool sendFrame(quint8 type, quint8 id, quint8 seq, const char *data, quint32 dataSize);
    Q_INVOKABLE bool sendFrame(quint8 type, quint8 id, const char* data, quint32 dataSize);

    Q_INVOKABLE bool sendCommand(ARCommandInfo *command, const QVariantMap &params);
    Q_INVOKABLE bool sendCommand(int projId, int classId, int commandId, const QVariantMap &params);

Q_SIGNALS:
    void error();

protected Q_SLOTS:
    void onReadyRead();

protected:
    void onPing(const ARControlFrame &frame);
    void onNavdata(const ARControlFrame &frame);
    void onVideoData(const ARControlFrame &frame);

private:
    class ARControlConnectionPrivate *d_ptr;
    Q_DECLARE_PRIVATE(ARControlConnection)
};

#endif // ARCONTROLCONNECTION_H
