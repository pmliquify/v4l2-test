#include <utils/errno.hpp>
#include <iostream>
#include <linux/videodev2.h>

const char * genericErrors(int err)
{
        switch (err) {
        case EAGAIN:    return "The ioctl can’t be handled because the device is in state where it can’t perform it. This could happen for example in case where device is sleeping and ioctl is performed to query statistics. It is also returned when the ioctl would need to wait for an event, but the device was opened in non-blocking mode.";
        case EBADF:     return "The file descriptor is not a valid.";
        case EBUSY:     return "The ioctl can’t be handled because the device is busy. This is typically return while device is streaming, and an ioctl tried to change something that would affect the stream, or would require the usage of a hardware resource that was already allocated. The ioctl must not be retried without performing another action to fix the problem first (typically: stop the stream before retrying).";
        case EFAULT:    return "There was a failure while copying data from/to userspace, probably caused by an invalid pointer reference.";
        case EINVAL:    return "One or more of the ioctl parameters are invalid or out of the allowed range. This is a widely used error code. See the individual ioctl requests for specific causes.";
        case ENODEV:    return "Device not found or was removed.";
        case ENOMEM:    return "There’s not enough memory to handle the desired operation.";
        case ENOTTY:    return "The ioctl is not supported by the driver, actually meaning that the required functionality is not available, or the file descriptor is not for a media device.";
        case ENOSPC:    return "On USB devices, the stream ioctl’s can return this error, meaning that this request would overcommit the usb bandwidth reserved for periodic transfers (up to 80% of the USB bandwidth).";
        case EPERM:     return "Permission denied. Can be returned if the device needs write permission, or some special capabilities is needed (e. g. root)";
        case EIO:       return "I/O error. Typically used when there are problems communicating with a hardware device. This could indicate broken or flaky hardware. It’s a ‘Something is wrong, I give up!’ type of error.";
        case ENXIO:     return "No device corresponding to this device special file exists.";
        case EMSGSIZE:  return "Message too long";
        default:        return "Unknown error code";
        }
}

const char * errorsForOpen(int err)
{
        switch (err) {
        case EACCES:    return "The caller has no permission to access the device.";
        case EBUSY:     return "The driver does not support multiple opens and the device is already in use.";
        case ENXIO:     return "No device corresponding to this device special file exists.";
        case ENOMEM:    return "Not enough kernel memory was available to complete the request.";
        case EMFILE:    return "The process already has the maximum number of files open.";
        case ENFILE:    return "The limit on the total number of files open on the system has been reached.";
        default:        return genericErrors(err);
        }
}

const char * errorsForClose(int err)
{
        switch (err) {
        case EBADF:     return "fd is not a valid open file descriptor.";
        default:        return genericErrors(err);
        }
}

const char * errorsForSelect(int err)
{
        switch (err) {
        case EBADF:     return "One or more of the file descriptor sets specified a file descriptor that is not open.";
        case EBUSY:     return "The driver does not support multiple read or write streams and the device is already in use.";
        case EFAULT:    return "The readfds, writefds, exceptfds or timeout pointer references an inaccessible memory area.";
        case EINTR:     return "The call was interrupted by a signal.";
        case EINVAL:    return "The nfds argument is less than zero or greater than FD_SETSIZE.";
        default:        return genericErrors(err);
        }
}

const char * errorsForIoctl(unsigned long int Request, int err)
{
        switch (Request) {
        case VIDIOC_QUERYCAP:
                // No specific errors.
                break;
        case VIDIOC_G_FMT: // Same as VIDIOC_S_FMT
        case VIDIOC_S_FMT:
                switch (err) {
                case EINVAL:    return "The struct v4l2_format type field is invalid or the requested buffer type not supported.";
                case EBUSY:     return "The device is busy and cannot change the format. This could be because or the device is streaming or buffers are allocated or queued to the driver. Relevant for VIDIOC_S_FMT only.";
                }
                break;
        case VIDIOC_S_SELECTION:
                switch (err) {
                case EINVAL:    return "Given buffer type 'type' or the selection target 'target' is not supported, or the flags argument is not valid.";
                case ERANGE:    return "It is not possible to adjust struct v4l2_rect r rectangle to satisfy all constraints given in the flags argument.";
                case ENODATA:   return "Selection is not supported for this input or output.";
                case EBUSY:     return "It is not possible to apply change of the selection rectangle at the moment. Usually because streaming is in progress.";
                }
                break;
        case VIDIOC_REQBUFS:
                switch (err) {
                case EINVAL:    return "The buffer type (type field) or the requested I/O method (memory) is not supported.";
                }
                break;
        case VIDIOC_QUERYBUF:
                switch (err) {
                case EINVAL:    return "The buffer type is not supported, or the index is out of bounds.";
                }
                break;
        case VIDIOC_QBUF: // Same as VIDIOC_DQBUF
        case VIDIOC_DQBUF:
                switch (err) {
                case EAGAIN:    return "Non-blocking I/O has been selected using O_NONBLOCK and no buffer was in the outgoing queue.";
                case EINVAL:    return "The buffer type is not supported, or the index is out of bounds, or no buffers have been allocated yet, or the userptr or length are invalid, or the V4L2_BUF_FLAG_REQUEST_FD flag was set but the the given request_fd was invalid, or m.fd was an invalid DMABUF file descriptor.";
                case EIO:       return "VIDIOC_DQBUF failed due to an internal error. Can also indicate temporary problems like signal loss.";
                case EPIPE:     return "VIDIOC_DQBUF returns this on an empty capture queue for mem2mem codecs if a buffer with the V4L2_BUF_FLAG_LAST was already dequeued and no new buffers are expected to become available.";
                case EBADR:     return "The V4L2_BUF_FLAG_REQUEST_FD flag was set but the device does not support requests for the given buffer type, or the V4L2_BUF_FLAG_REQUEST_FD flag was not set but the device requires that the buffer is part of a request.";
                case EBUSY:     return "The first buffer was queued via a request, but the application now tries to queue it directly, or vice versa (it is not permitted to mix the two APIs).";
                }
                break;
        case VIDIOC_STREAMON:
                switch (err) {
                case EINVAL:    return "The buffer type is not supported, or no buffers have been allocated (memory mapping) or enqueued (output) yet";
                case EPIPE:     return "The driver implements pad-level format configuration and the pipeline configuration is invalid.";
                case ENOLINK:   return "The driver implements Media Controller interface and the pipeline link configuration is invalid.";
                }
                break;
        case VIDIOC_STREAMOFF:
                switch (err) {
                case EINVAL:    return "The buffer type is not supported, or no buffers have been allocated (memory mapping) or enqueued (output) yet.";
                case EPIPE:     return "The driver implements pad-level format configuration and the pipeline configuration is invalid.";
                case ENOLINK:   return "The driver implements Media Controller interface and the pipeline link configuration is invalid.";
                }
                break;
        case VIDIOC_S_CTRL:
                break;
        }
        return genericErrors(err);
}

const char * errorsForRecv(int err)
{
        switch (err) {
        case EAGAIN:      return "The socket's file descriptor is marked O_NONBLOCK and no data is waiting to be received; or MSG_OOB is set and no out-of-band data is available and either the socket's file descriptor is marked O_NONBLOCK or the socket does not support blocking to await out-of-band data.";
        case EBADF:       return "The socket argument is not a valid file descriptor.";
        case ECONNRESET:  return "A connection was forcibly closed by a peer.";
        case EFAULT:      return "The buffer parameter can not be accessed or written.";
        case EINTR:       return "The recv() function was interrupted by a signal that was caught, before any data was available.";
        case EINVAL:      return "The MSG_OOB flag is set and no out-of-band data is available.";
        case ENOTCONN:    return "A receive is attempted on a connection-mode socket that is not connected.";
        case ENOTSOCK:    return "The socket argument does not refer to a socket.";
        case EOPNOTSUPP:  return "The specified flags are not supported for this socket type or protocol.";
        case ETIMEDOUT:   return "The connection timed out during connection establishment, or due to a transmission timeout on active connection.";
        case EIO:         return "An I/O error occurred while reading from or writing to the file system.";
        case ENOBUFS:     return "Insufficient resources were available in the system to perform the operation.";
        case ENOMEM:      return "Insufficient memory was available to fulfill the request.";
        case ENOSR:       return "There were insufficient STREAMS resources available for the operation to complete.";
        default:          return "";
        }
}