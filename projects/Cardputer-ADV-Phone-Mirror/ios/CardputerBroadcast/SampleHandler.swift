import ReplayKit
import Network
import CoreImage
import ImageIO
import UIKit
import QuartzCore

final class FrameSender {
    private let queue = DispatchQueue(label: "CardputerMirror.FrameSender")
    private var connection: NWConnection?
    private var ready = false
    private var sending = false

    private let host = NWEndpoint.Host("192.168.4.1")
    private let port = NWEndpoint.Port(rawValue: 9000)!

    func start() {
        queue.async { self.connect() }
    }

    func stop() {
        queue.async {
            self.connection?.cancel()
            self.connection = nil
            self.ready = false
            self.sending = false
        }
    }

    private func connect() {
        connection?.cancel()
        let c = NWConnection(host: host, port: port, using: .tcp)
        connection = c
        c.stateUpdateHandler = { [weak self] state in
            guard let self else { return }
            self.queue.async {
                switch state {
                case .ready:
                    self.ready = true
                case .failed, .cancelled:
                    self.ready = false
                    self.sending = false
                default:
                    break
                }
            }
        }
        c.start(queue: queue)
    }

    func send(jpeg: Data) {
        queue.async {
            guard self.ready, !self.sending, let c = self.connection else { return }
            self.sending = true

            var packet = Data("CMIR".utf8)
            var length = UInt32(jpeg.count).bigEndian
            withUnsafeBytes(of: &length) { packet.append(contentsOf: $0) }
            packet.append(jpeg)

            c.send(content: packet, completion: .contentProcessed { [weak self] error in
                guard let self else { return }
                self.queue.async {
                    self.sending = false
                    if error != nil {
                        self.ready = false
                        self.connect()
                    }
                }
            })
        }
    }
}

final class SampleHandler: RPBroadcastSampleHandler {
    private let sender = FrameSender()
    private let ciContext = CIContext(options: [.cacheIntermediates: false])
    private var lastFrameTime: CFTimeInterval = 0
    private let maxFPS: CFTimeInterval = 8.0
    private let targetWidth: CGFloat = 240
    private let targetHeight: CGFloat = 135

    override func broadcastStarted(withSetupInfo setupInfo: [String : NSObject]?) {
        sender.start()
    }

    override func broadcastPaused() {}
    override func broadcastResumed() {}

    override func broadcastFinished() {
        sender.stop()
    }

    override func processSampleBuffer(_ sampleBuffer: CMSampleBuffer,
                                      with sampleBufferType: RPSampleBufferType) {
        guard sampleBufferType == .video else { return }

        let now = CACurrentMediaTime()
        guard now - lastFrameTime >= 1.0 / maxFPS else { return }
        lastFrameTime = now

        guard let pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer) else { return }
        autoreleasepool {
            guard let jpeg = makeJPEG(pixelBuffer: pixelBuffer, sampleBuffer: sampleBuffer) else { return }
            sender.send(jpeg: jpeg)
        }
    }

    private func makeJPEG(pixelBuffer: CVPixelBuffer,
                          sampleBuffer: CMSampleBuffer) -> Data? {
        var image = CIImage(cvPixelBuffer: pixelBuffer)

        if let rawOrientation = CMGetAttachment(
            sampleBuffer,
            key: RPVideoSampleOrientationKey as CFString,
            attachmentModeOut: nil
        ) as? NSNumber,
           let orientation = CGImagePropertyOrientation(rawValue: rawOrientation.uint32Value) {
            image = image.oriented(orientation)
        }

        let extent = image.extent.integral
        guard extent.width > 0, extent.height > 0 else { return nil }

        let targetAspect = targetWidth / targetHeight
        let sourceAspect = extent.width / extent.height

        let crop: CGRect
        if sourceAspect > targetAspect {
            let width = extent.height * targetAspect
            crop = CGRect(x: extent.midX - width / 2,
                          y: extent.minY,
                          width: width,
                          height: extent.height)
        } else {
            let height = extent.width / targetAspect
            crop = CGRect(x: extent.minX,
                          y: extent.midY - height / 2,
                          width: extent.width,
                          height: height)
        }

        let cropped = image.cropped(to: crop)
        let translated = cropped.transformed(by: CGAffineTransform(
            translationX: -crop.minX,
            y: -crop.minY
        ))
        let scaled = translated.transformed(by: CGAffineTransform(
            scaleX: targetWidth / crop.width,
            y: targetHeight / crop.height
        ))

        let outputRect = CGRect(x: 0, y: 0, width: targetWidth, height: targetHeight)
        guard let cg = ciContext.createCGImage(scaled, from: outputRect) else { return nil }
        return UIImage(cgImage: cg).jpegData(compressionQuality: 0.45)
    }
}
