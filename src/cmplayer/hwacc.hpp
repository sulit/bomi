#ifndef HWACC_HPP
#define HWACC_HPP

#include "stdafx.hpp"
extern "C" {
#include <libavcodec/avcodec.h>
#include <video/img_format.h>
}

struct lavc_ctx;		struct AVFrame;
struct mp_image;		struct vd_lavc_hwdec;
struct mp_hwdec_info;	class VideoOutput;

template<typename T>
struct HwAccCodec {
	HwAccCodec() {}
	HwAccCodec(const QVector<T> &available, const QVector<T> &p, const QVector<int> &av, int surfaces, AVCodecID id) {
		Q_ASSERT(p.size() == av.size());
		for (int i=0; i<p.size(); ++i) {
			if (available.contains(p[i])) {
				profiles.push_back(p[i]);
				avProfiles.push_back(av[i]);
			}
		}
		if (!profiles.isEmpty()) {
			this->surfaces = surfaces;
			this->id = id;
		}
	}
	T profile(int av) const {
		const int idx = avProfiles.indexOf(av);
		return profiles[idx < 0 || av == FF_PROFILE_UNKNOWN ? 0 : idx];
	}
	AVCodecID id = AV_CODEC_ID_NONE;
	int surfaces = 0, level = 0;
	QVector<T> profiles;
	QVector<int> avProfiles;
};

class HwAcc {
public:
	enum Type {VaApiGLX, VaApiX11, VdpauX11, Vda};
	virtual ~HwAcc();
	static void initialize();
	static void finalize();
	static QList<AVCodecID> fullCodecList();
	static bool supports(AVCodecID codec);
	static const char *codecName(AVCodecID id);
	static const char *name();
	virtual mp_image *getImage(mp_image *mpi) = 0;
	virtual Type type() const = 0;
	mp_image *nullImage(uint imgfmt, int width, int height, void *arg = nullptr, void(*free)(void*) = nullptr) const;
	int imgfmt() const;
protected:
	HwAcc(AVCodecID codec);
	virtual bool isOk() const = 0;
	virtual mp_image *getSurface() = 0;
	virtual void *context() const = 0;
	virtual bool fillContext(AVCodecContext *avctx) = 0;
	AVCodecID codec() const {return m_codec;}
	const QSize &size() const {return m_size;}
	QSize &size() {return m_size;}
private:
	static VideoOutput *vo(lavc_ctx *ctx);
	static int probe(vd_lavc_hwdec *hwdec, mp_hwdec_info *info, const char *decoder);
	static int init(lavc_ctx *ctx);
	static void uninit(lavc_ctx *ctx);
	static mp_image *allocateImage(struct lavc_ctx *ctx, int imgfmt, int width, int height);
	friend class PlayEngine;
	friend vd_lavc_hwdec create_vaapi_functions();
	friend vd_lavc_hwdec create_vdpau_functions();
	friend vd_lavc_hwdec create_vda_functions();
	struct Data;
	Data *d;
	AVCodecID m_codec = AV_CODEC_ID_NONE;
	QSize m_size = {0,0};
};

#endif // HWACC_HPP
