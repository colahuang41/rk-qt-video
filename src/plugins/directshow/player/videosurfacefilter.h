/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef VIDEOSURFACEFILTER_H
#define VIDEOSURFACEFILTER_H

#include "directshowglobal.h"
#include "directshowmediatypelist.h"
#include "directshowsamplescheduler.h"
#include "directshowmediatype.h"

#include <QtCore/qbasictimer.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qmutex.h>
#include <QtCore/qsemaphore.h>
#include <QtCore/qstring.h>
#include <QtCore/qwaitcondition.h>

#include <dshow.h>

QT_BEGIN_NAMESPACE
class QAbstractVideoSurface;
QT_END_NAMESPACE

class DirectShowEventLoop;

class VideoSurfaceFilter
    : public QObject
    , public DirectShowMediaTypeList
    , public IBaseFilter
    , public IAMFilterMiscFlags
    , public IPin
{
    Q_OBJECT
public:
    VideoSurfaceFilter(
            QAbstractVideoSurface *surface, DirectShowEventLoop *loop, QObject *parent = 0);
    ~VideoSurfaceFilter();

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    // IPersist
    HRESULT STDMETHODCALLTYPE GetClassID(CLSID *pClassID);

    // IMediaFilter
    HRESULT STDMETHODCALLTYPE Run(REFERENCE_TIME tStart);
    HRESULT STDMETHODCALLTYPE Pause();
    HRESULT STDMETHODCALLTYPE Stop();

    HRESULT STDMETHODCALLTYPE GetState(DWORD dwMilliSecsTimeout, FILTER_STATE *pState);

    HRESULT STDMETHODCALLTYPE SetSyncSource(IReferenceClock *pClock);
    HRESULT STDMETHODCALLTYPE GetSyncSource(IReferenceClock **ppClock);

    // IBaseFilter
    HRESULT STDMETHODCALLTYPE EnumPins(IEnumPins **ppEnum);
    HRESULT STDMETHODCALLTYPE FindPin(LPCWSTR Id, IPin **ppPin);

    HRESULT STDMETHODCALLTYPE JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName);

    HRESULT STDMETHODCALLTYPE QueryFilterInfo(FILTER_INFO *pInfo);
    HRESULT STDMETHODCALLTYPE QueryVendorInfo(LPWSTR *pVendorInfo);

    // IAMFilterMiscFlags
    ULONG STDMETHODCALLTYPE GetMiscFlags();

    // IPin
    HRESULT STDMETHODCALLTYPE Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);
    HRESULT STDMETHODCALLTYPE ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt);
    HRESULT STDMETHODCALLTYPE Disconnect();
    HRESULT STDMETHODCALLTYPE ConnectedTo(IPin **ppPin);

    HRESULT STDMETHODCALLTYPE ConnectionMediaType(AM_MEDIA_TYPE *pmt);

    HRESULT STDMETHODCALLTYPE QueryPinInfo(PIN_INFO *pInfo);
    HRESULT STDMETHODCALLTYPE QueryId(LPWSTR *Id);

    HRESULT STDMETHODCALLTYPE QueryAccept(const AM_MEDIA_TYPE *pmt);

    HRESULT STDMETHODCALLTYPE EnumMediaTypes(IEnumMediaTypes **ppEnum);

    HRESULT STDMETHODCALLTYPE QueryInternalConnections(IPin **apPin, ULONG *nPin);

    HRESULT STDMETHODCALLTYPE EndOfStream();

    HRESULT STDMETHODCALLTYPE BeginFlush();
    HRESULT STDMETHODCALLTYPE EndFlush();

    HRESULT STDMETHODCALLTYPE NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

    HRESULT STDMETHODCALLTYPE QueryDirection(PIN_DIRECTION *pPinDir);

    int currentMediaTypeToken();
    HRESULT nextMediaType(
            int token, int *index, ULONG count, AM_MEDIA_TYPE **types, ULONG *fetchedCount);
    HRESULT skipMediaType(int token, int *index, ULONG count);
    HRESULT cloneMediaType(int token, int index, IEnumMediaTypes **enumeration);

protected:
    void customEvent(QEvent *event);

private Q_SLOTS:
    void supportedFormatsChanged();
    void sampleReady();

private:
    HRESULT start();
    void stop();
    void flush();

    enum
    {
        StartSurface = QEvent::User,
        StopSurface,
        FlushSurface
    };

    LONG m_ref;
    FILTER_STATE m_state;
    QAbstractVideoSurface *m_surface;
    DirectShowEventLoop *m_loop;
    IFilterGraph *m_graph;
    IPin *m_peerPin;
    int m_bytesPerLine;
    HRESULT m_startResult;
    QString m_name;
    QString m_pinId;
    DirectShowMediaType m_mediaType;
    QVideoSurfaceFormat m_surfaceFormat;
    QMutex m_mutex;
    QWaitCondition m_wait;
    DirectShowSampleScheduler m_sampleScheduler;
};

#endif
