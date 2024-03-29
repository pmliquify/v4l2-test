#include <sources/v4l2imagesource.hpp>
#include <utils/errno.hpp>
#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>


V4L2ImageSource::V4L2ImageSource() :
        m_image(NULL),
        m_deviceFd(0),
        m_subDeviceFd(0),
        m_buffers(NULL),
        m_bufferCount(0),
        m_nextBufferIndex(0)
{
        m_image = new V4L2Image();
}

V4L2ImageSource::~V4L2ImageSource()
{
        close();

        delete m_image;
        m_image = NULL;
}

void V4L2ImageSource::printArgs()
{
        printArgSection("V4L2 ImageSource");
        printArg("-d",      "Set device /dev/video0");
        printArg("-sd",     "Set subdevice /dev/v4l-subdev0");
        printArg("-bi",     "Set binning (see binning modes of each camera module)");
        printArg("-bl",     "Set black level");
        printArg("-e",      "Set exposure time");
        printArg("-f",      "Set pixel format");
        printArg("-g",      "Set gain value");
        printArg("-m",      "Set IO mode");
        printArg("-r",      "Set frame rate");
        printArg("-t",      "Set trigger mode");
        printArg("--shift", "Set bit shift for each pixel in RAW10, RAW12 format");
}

int V4L2ImageSource::setup(CommandArgs &args)
{
        open(args.option("-d", "/dev/video0"), args.option("-sd", ""));
        printFormat();
        
        if (args.exists("-bi")) {
                setBinning(args.optionInt("-bi"));
        }
        if (args.exists("-bl")) {
                setBlackLevel(args.optionInt("-bl"));
        }
        if (args.exists("-e")) {
                setExposure(args.optionInt("-e"));
        }
        if (args.exists("-f")) {
                std::string format = args.option("-f");
                if (format.size() == 4) {
                        setFormat(*(int *)format.c_str());
                }
        } else {
                setFormat();
        }
        if (args.exists("-g")) {
                setGain(args.optionInt("-g"));
        }
        if (args.exists("-m")) {
                setIOMode(args.optionInt("-m"));
        }
        if (args.exists("-r")) {
                setFrameRate(args.optionInt("-r"));
        }
        if (args.exists("-t")) {
                setTriggerMode(args.optionInt("-t"));
        }
        m_image->setShift( args.optionInt("--shift", 0));
        
        return 0;
}

int V4L2ImageSource::open(const std::string devicePath, const std::string subDevicePath)
{
        m_deviceFd = ::open(devicePath.c_str(), O_RDWR, 0);
        if (-1 == m_deviceFd) {
                handleErrorForOpen(devicePath.c_str(), errno);
                return -1;
        }

        struct v4l2_capability capDevice;
        memset(&capDevice, 0, sizeof(capDevice));
        if (-1 == ioctl(m_deviceFd, VIDIOC_QUERYCAP, &capDevice)) {
                handleErrorForIoctl(VIDIOC_QUERYCAP, errno);
                return -1;
        }
        if (!(capDevice.capabilities & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING))) {
                return -1;
        }

        const char *subdevicePath = NULL;
        if (subDevicePath.empty()) {
                m_subDeviceFd = m_deviceFd;

        } else {
                subdevicePath = subDevicePath.c_str();
                m_subDeviceFd = ::open(subdevicePath, O_RDWR, 0);
                if (-1 == m_deviceFd) {
                        handleErrorForOpen(subdevicePath, errno);
                        return -1;
                }
        }
        
        return 0;
}

int V4L2ImageSource::close()
{
        clearBuffers();

        if (m_deviceFd != 0 && -1 == ::close(m_deviceFd)) {
                handleErrorForClose(m_deviceFd, errno);
        }
        if (m_deviceFd != m_subDeviceFd) {
                if (-1 == ::close(m_subDeviceFd)) {
                        handleErrorForClose(m_subDeviceFd, errno);
                }
        }
        m_deviceFd = 0;
        m_subDeviceFd = 0;
        return 0;
}

int V4L2ImageSource::getFormat()
{
        memset(&m_format, 0, sizeof(m_format));
        m_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (0 == ioctl(m_deviceFd, VIDIOC_G_FMT, &m_format)) {
                return 0;
        }
        m_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        if (0 == ioctl(m_deviceFd, VIDIOC_G_FMT, &m_format)) {
                return 0;
        }
        handleErrorForIoctl(VIDIOC_G_FMT, errno);
        return -1;
}

int V4L2ImageSource::setFormat(int pixelFormat)
{
        int ret = getFormat();
        if (ret == -1) {
                return -1;
        }  

        if (pixelFormat != 0) {
                switch (m_format.type) {
                case V4L2_BUF_TYPE_VIDEO_CAPTURE:
                        m_format.fmt.pix.pixelformat = pixelFormat;
                        break;

                case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
                        m_format.fmt.pix_mp.pixelformat = pixelFormat;
                        break;
                }
        }
        if (-1 == ioctl(m_deviceFd, VIDIOC_S_FMT, &m_format)) {
                handleErrorForIoctl(VIDIOC_S_FMT, errno);
                return -1;
        } 
        return 0;
}

int V4L2ImageSource::printFormat()
{
        int ret = getFormat();
        if (ret == -1) {
                return -1;
        }  

        char pixelFormat[5];
        switch (m_format.type) {
        case V4L2_BUF_TYPE_VIDEO_CAPTURE:
                memcpy(&pixelFormat, (const void*)&m_format.fmt.pix.pixelformat, 4);
                pixelFormat[4] = 0;

                std::cout << "Format (width: " << m_format.fmt.pix.width
                        << ", height: " << m_format.fmt.pix.height
                        << ", pixelformat: " << pixelFormat
                        << ", colorspace: ";
                switch (m_format.fmt.pix.colorspace) {
                case V4L2_COLORSPACE_SRGB:
                        std::cout << "SRGB";
                        break;
                case V4L2_COLORSPACE_RAW:
                        std::cout << "RAW";
                        break;
                }
                std::cout << ")" << std::endl;
                break;

        case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
                memcpy(&pixelFormat, (const void*)&m_format.fmt.pix_mp.pixelformat, 4);
                pixelFormat[4] = 0;

                std::cout << "Format (width: " << m_format.fmt.pix_mp.width
                        << ", height: " << m_format.fmt.pix_mp.height
                        << ", pixelformat: " << pixelFormat
                        << ", colorspace: ";
                switch (m_format.fmt.pix_mp.colorspace) {
                case V4L2_COLORSPACE_SRGB:
                        std::cout << "SRGB";
                        break;
                case V4L2_COLORSPACE_RAW:
                        std::cout << "RAW";
                        break;
                }
                std::cout << ")" << std::endl;
                break;
        }
        return 0;
}

int V4L2ImageSource::setSelection(int left, int top, int width, int height)
{
        switch (m_format.type) {
        case V4L2_BUF_TYPE_VIDEO_CAPTURE:
                width = (width <= 0) ? m_format.fmt.pix.width : width;
                height = (height <= 0) ? m_format.fmt.pix.height : height;
                break;
        case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
                width = (width <= 0) ? m_format.fmt.pix_mp.width : width;
                height = (height <= 0) ? m_format.fmt.pix_mp.height : height;
                break;
        }

        struct v4l2_selection selection;
        memset(&selection, 0, sizeof(selection));
        selection.type = m_format.type;
        selection.target = V4L2_SEL_TGT_CROP;
        selection.r.left = left;
        selection.r.top = top;
        selection.r.width = width;
        selection.r.height = height;
        selection.flags = V4L2_SEL_FLAG_LE | V4L2_SEL_FLAG_GE;
        if (-1 == ioctl(m_deviceFd, VIDIOC_S_SELECTION, &selection)) {
                handleErrorForIoctl(VIDIOC_S_SELECTION, errno);
                return -1;
        } 
        return 0;
}

int V4L2ImageSource::streamOn(int bufferCount)
{
        int ret = getFormat();
        if (ret == -1) {
                return -1;
        }  
        
        ret = initBuffers(bufferCount);
        if (ret != 0) {
                return -1;
        }  

        if (-1 == ioctl(m_deviceFd, VIDIOC_STREAMON, &m_format.type)) {
                handleErrorForIoctl(VIDIOC_STREAMON, errno);
                return -1;
        }
        return 0;
}

int V4L2ImageSource::streamOff()
{
        if (-1 == ioctl(m_deviceFd, VIDIOC_STREAMOFF, &m_format.type)) {
                handleErrorForIoctl(VIDIOC_STREAMOFF, errno);
                return -1;
        }
        return 0;
}

Image *V4L2ImageSource::getNextImage(int timeout, bool lastImage)
{
        if (lastImage) {
                int ret = 0;
                while(ret == 0) {
                        ret = waitForNextBuffer(0);
                        if (ret == 0) {
                                dequeueBuffer(m_nextBufferIndex);
                                enqueueBuffer(m_nextBufferIndex);
                                m_nextBufferIndex = (m_nextBufferIndex + 1) % m_bufferCount;
                        }
                }
        } 
        if (-1 == waitForNextBuffer(timeout)) {
                return NULL;
        }

        struct v4l2_buffer *buffer = dequeueBuffer(m_nextBufferIndex);
        if (buffer == NULL) {
                return NULL;
        }

        m_image->setBufferIndex(m_nextBufferIndex);
        u_int64_t timestamp = buffer->timestamp.tv_sec*1e3 + buffer->timestamp.tv_usec/1e3;

        switch (m_format.type) {
        case V4L2_BUF_TYPE_VIDEO_CAPTURE:
                m_image->init(m_format.fmt.pix.width, m_format.fmt.pix.height, m_format.fmt.pix.bytesperline,
                        m_format.fmt.pix.sizeimage, buffer->bytesused, m_format.fmt.pix.pixelformat, 
                        buffer->sequence, timestamp);
                m_image->planes().resize(1);
                m_image->planes()[0] = m_buffers[m_nextBufferIndex].ptrs[0];
                break;

        case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
                m_image->init(m_format.fmt.pix_mp.width, m_format.fmt.pix_mp.height, m_format.fmt.pix_mp.plane_fmt->bytesperline,
                        m_format.fmt.pix_mp.plane_fmt->sizeimage, buffer->bytesused, m_format.fmt.pix_mp.pixelformat, 
                        buffer->sequence, timestamp);
                m_image->planes().resize(buffer->length);
                for (unsigned int planeIndex = 0; planeIndex < buffer->length; planeIndex++) {
                        m_image->planes()[planeIndex] = m_buffers[m_nextBufferIndex].ptrs[planeIndex];
                }
                break;
        }
        
        return m_image;
}

int V4L2ImageSource::releaseImage(Image *image)
{
        if (image != m_image) {
                return -1;
        }

        if (-1 == enqueueBuffer(m_image->bufferIndex())) {
                return -1;
        }

        m_nextBufferIndex = (m_nextBufferIndex + 1) % m_bufferCount;
        return 0;
}

Image *V4L2ImageSource::getImage(int timeout, bool lastImage)
{
        if (-1 == streamOn(3)) {
                return NULL;
        }
        if (NULL == getNextImage(timeout, lastImage)) {
                releaseImage(m_image);
        }
        streamOff();
        return m_image;
}

int V4L2ImageSource::setExposure(int exposure)
{
        return setControl("Exposure", exposure);
}

int V4L2ImageSource::setGain(int gain)
{
        return setControl("Gain", gain);
}

int V4L2ImageSource::setBlackLevel(int blackLevel)
{
        return setControl("Black Level", blackLevel);
}

int V4L2ImageSource::setBinning(int binning)
{
        return setControl("Binning", binning);
}

int V4L2ImageSource::setTriggerMode(int triggerMode)
{
        return setControl("Trigger Mode", triggerMode);
}

int V4L2ImageSource::setIOMode(int ioMode)
{
        return setControl("IO Mode", ioMode);
}

int V4L2ImageSource::setFrameRate(int frameRate)
{
        return setControl("Frame Rate", frameRate);
}

int V4L2ImageSource::setControl(unsigned int id, int value)
{
        struct v4l2_control control;
        control.id = id;
        control.value = value;
        if (-1 == ioctl(m_deviceFd, VIDIOC_S_CTRL, &control)) {
                handleErrorForIoctl(VIDIOC_S_CTRL, errno);
                return -1;
        }
        return 0;
}

int V4L2ImageSource::setExtControl(unsigned int id, unsigned int type, int value)
{
	struct v4l2_ext_control  ext_control;
        memset(&ext_control, 0, sizeof(ext_control));
        ext_control.id = id;
        switch (type) {
        case V4L2_CTRL_TYPE_INTEGER:   ext_control.value = value;   break;
        case V4L2_CTRL_TYPE_INTEGER64: ext_control.value64 = value; break;
        }

        struct v4l2_ext_controls ext_controls;
        ext_controls.ctrl_class = type;
        ext_controls.count      = 1;
        ext_controls.controls   = &ext_control;
        if (-1 == ioctl(m_subDeviceFd, VIDIOC_S_EXT_CTRLS, &ext_controls)) {
                handleErrorForIoctl(VIDIOC_S_EXT_CTRLS, errno);
                return -1;
        }
        return 0;
}

int V4L2ImageSource::setControl(std::string name, int value)
{
        struct v4l2_queryctrl  queryctrl;
        queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
        while(0 == ioctl(m_subDeviceFd, VIDIOC_QUERYCTRL, &queryctrl)) {
                if (0 == name.compare((const char *)queryctrl.name)) {
                        // printf("Control (name: '%s', id: 0x%08x, type: %u, flags: 0x%08x, value: %u)\n", 
                        //         name.c_str(), queryctrl.id, queryctrl.type, queryctrl.flags, value);
                        return setExtControl(queryctrl.id, queryctrl.type, value);
                }
                queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
        }
        printf("Control (name: '%s') not found!\n", name.c_str());
        return -1;
}

int V4L2ImageSource::initBuffers(int bufferCount)
{
        clearBuffers();

        struct v4l2_requestbuffers request;
        memset(&request, 0, sizeof(request));
        request.type = m_format.type;
        request.memory = V4L2_MEMORY_MMAP;
        request.count = bufferCount;
        if (-1 == ioctl(m_deviceFd, VIDIOC_REQBUFS, &request)) {
                handleErrorForIoctl(VIDIOC_REQBUFS, errno);
                return -1;
        }
        // Für NVIDIA =>= request.capabilities = 0
        // if (!request.capabilities & V4L2_BUF_CAP_SUPPORTS_MMAP) {
        //         return -1;
        // }

        int planeCount = 1;
        if (m_format.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
                planeCount = m_format.fmt.pix_mp.num_planes;
        }
        m_bufferCount = request.count;
        m_buffers = (struct Buffer *)malloc(m_bufferCount*sizeof(struct Buffer));
        for (int bufferIndex = 0; bufferIndex < m_bufferCount; bufferIndex++) {
                struct v4l2_buffer *buffer = &m_buffers[bufferIndex].buffer;
                memset(buffer, 0, sizeof(struct v4l2_buffer));
                switch (m_format.type) {
                case V4L2_BUF_TYPE_VIDEO_CAPTURE:        
                        m_buffers[bufferIndex].ptrs = (u_int8_t **)malloc(sizeof(void *));
                        m_buffers[bufferIndex].ptrs[0] = NULL;
                        break;

                case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
                        buffer->length = planeCount;
                        buffer->m.planes = (struct v4l2_plane *)malloc(buffer->length*sizeof(struct v4l2_plane));
                        memset(buffer->m.planes, 0, buffer->length*sizeof(struct v4l2_plane));
                        m_buffers[bufferIndex].ptrs = (u_int8_t **)malloc(buffer->length*sizeof(void *));
                        memset(m_buffers[bufferIndex].ptrs, 0, buffer->length*sizeof(void *));
                        break;
                }
        }
        
        for (int bufferIndex = 0; bufferIndex < m_bufferCount; bufferIndex++) {
                struct v4l2_buffer *buffer = &m_buffers[bufferIndex].buffer;
                buffer->type = m_format.type;
                buffer->memory = V4L2_MEMORY_MMAP;
                buffer->index = bufferIndex;
                int ret = ioctl(m_deviceFd, VIDIOC_QUERYBUF, buffer);
                if (ret != 0) {
                        return -1;
                }

                switch (m_format.type) {
                case V4L2_BUF_TYPE_VIDEO_CAPTURE:
                        {
                                u_int8_t *ptr = NULL;
                                ptr = (u_int8_t *)mmap(NULL, buffer->length, PROT_READ | PROT_WRITE, 
                                        MAP_SHARED, m_deviceFd, buffer->m.offset);
                                if (ptr == MAP_FAILED) {
                                        return -1;
                                }
                                m_buffers[bufferIndex].ptrs[0] = ptr;
                        }
                        break;

                case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
                        for (int planeIndex = 0; planeIndex < planeCount; planeIndex++) {
                                struct v4l2_plane *plane = &buffer->m.planes[planeIndex];
                                u_int8_t *ptr = NULL;
                                ptr = (u_int8_t *)mmap(NULL, plane->length, PROT_READ | PROT_WRITE,
                                        MAP_SHARED, m_deviceFd, plane->m.mem_offset);
                                if (ptr == MAP_FAILED) {
                                        return -1;
                                }
                                m_buffers[bufferIndex].ptrs[planeIndex] = ptr;
                        }
                        break;
                }
        }

        for (int bufferIndex = 0; bufferIndex < m_bufferCount; bufferIndex++) {
                if (-1 == enqueueBuffer(bufferIndex)) {
                        return -1;
                }
        }
        m_nextBufferIndex = 0;

        return 0;
}

void V4L2ImageSource::clearBuffers()
{
        if (m_buffers == NULL) {
                return;
        }

        for (int bufferIndex = 0; bufferIndex < m_bufferCount; bufferIndex++) {
                struct v4l2_buffer *buffer = &m_buffers[bufferIndex].buffer;
                switch (m_format.type) {
                case V4L2_BUF_TYPE_VIDEO_CAPTURE:
                        break;
                case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
                        free(buffer->m.planes); 
                        break;
                }
                free(m_buffers[bufferIndex].ptrs);
        }
        free(m_buffers);
        m_buffers = NULL;
        m_bufferCount = 0;
}

int V4L2ImageSource::enqueueBuffer(int bufferIndex)
{
        struct v4l2_buffer *buffer = &m_buffers[bufferIndex].buffer;
        buffer->flags = 0;
	if (-1 == ioctl(m_deviceFd, VIDIOC_QBUF, buffer)) {
                handleErrorForIoctl(VIDIOC_QBUF, errno);
                return -1;
        }
        if (!(buffer->flags & (V4L2_BUF_FLAG_MAPPED | V4L2_BUF_FLAG_QUEUED))) {
                return -1;
        }
        return 0;
}

struct v4l2_buffer * V4L2ImageSource::dequeueBuffer(int bufferIndex)
{
        struct v4l2_buffer *buffer = &m_buffers[bufferIndex].buffer;
        buffer->flags = 0;
	if (-1 == ioctl(m_deviceFd, VIDIOC_DQBUF, buffer)) {
                handleErrorForIoctl(VIDIOC_DQBUF, errno);
                return NULL;
        }
	if(buffer->flags & V4L2_BUF_FLAG_QUEUED) {
                return NULL;
	}
        return buffer;
}

int V4L2ImageSource::waitForNextBuffer(int timeout)
{
	while(true) {
                fd_set set;
		FD_ZERO(&set);
		FD_SET(m_deviceFd, &set);
                struct timeval timeval;
		timeval.tv_sec  = (timeout/1000000);
		timeval.tv_usec = (timeout%1000000);

		int ret = select(m_deviceFd + 1, &set, NULL, NULL, &timeval);
                if (ret > 0) {
                        // Success
                        return 0;
                }
                if (ret == 0) {
                        // Abort because of timeout.
                        return -2;
                } 
                if (ret < 0) {
                        switch (errno) {
                        case EINTR:
                                // Ignore interrupt based returns.
                                break;
                        default:
                                handleErrorForSelect(m_deviceFd, errno);
                                return -1;
                        }
                } 
	}
}

void V4L2ImageSource::handleErrorForOpen(const char *path, int err)
{
        char message[32];
        sprintf(message, "open (path: %s):", path);
        perror(message);
        printf("%s\n", errorsForOpen(err));
}

void V4L2ImageSource::handleErrorForClose(int fd, int err)
{
        char message[32];
        sprintf(message, "close (fd: %d):", fd);
        perror(message);
        printf("%s\n", errorsForClose(err));
}

void V4L2ImageSource::handleErrorForSelect(int fd, int err)
{
        char message[32];
        sprintf(message, "select (fd: %d):", fd);
        perror(message);
        printf("%s\n", errorsForSelect(err));
}

void V4L2ImageSource::handleErrorForIoctl(unsigned long int request, int err)
{
        switch (request) {
        case VIDIOC_QUERYCAP:    perror("VIDIOC_QUERYCAP");       break;
        case VIDIOC_G_FMT:       perror("VIDIOC_G_FMT");          break;
        case VIDIOC_S_FMT:       perror("VIDIOC_S_FMT");          break;
        case VIDIOC_S_SELECTION: perror("VIDIOC_S_SELECTION");    break;
        case VIDIOC_REQBUFS:     perror("VIDIOC_REQBUFS");        break;
        case VIDIOC_QUERYBUF:    perror("VIDIOC_QUERYBUF");       break;
        case VIDIOC_QBUF:        perror("VIDIOC_QBUF");           break;
        case VIDIOC_DQBUF:       perror("VIDIOC_DQBUF");          break;
        case VIDIOC_STREAMON:    perror("VIDIOC_STREAMON");       break;
        case VIDIOC_STREAMOFF:   perror("VIDIOC_STREAMOFF");      break;
        case VIDIOC_S_CTRL:      perror("VIDIOC_S_CTRL");         break;
        case VIDIOC_S_EXT_CTRLS: perror("VIDIOC_S_EXT_CTRLS");    break;
        default:                 perror("Unknown ioctl request"); break;
        }
        printf("%s\n", errorsForIoctl(request, err));
}