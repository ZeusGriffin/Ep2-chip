import SwiftUI

struct ContentView: View {
    var body: some View {
        NavigationStack {
            VStack(spacing: 18) {
                Spacer()

                Image(systemName: "rectangle.on.rectangle.angled")
                    .font(.system(size: 58))

                Text("Cardputer Mirror")
                    .font(.largeTitle.bold())

                Text("Mirror your iPhone screen to the Cardputer ADV over Wi-Fi.")
                    .multilineTextAlignment(.center)
                    .foregroundStyle(.secondary)

                VStack(alignment: .leading, spacing: 8) {
                    Label("Connect iPhone Wi-Fi to CardputerMirror", systemImage: "wifi")
                    Label("Password: cardputer", systemImage: "key")
                    Label("Tap Start Broadcast below", systemImage: "record.circle")
                    Label("Then leave this app and use your iPhone normally", systemImage: "iphone")
                }
                .font(.subheadline)
                .frame(maxWidth: .infinity, alignment: .leading)
                .padding()
                .background(.thinMaterial, in: RoundedRectangle(cornerRadius: 16))

                BroadcastPicker()
                    .frame(width: 62, height: 62)
                    .background(.thinMaterial, in: RoundedRectangle(cornerRadius: 16))

                Text("The system broadcast control above is required by iOS. Secure/DRM video may appear blank.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .multilineTextAlignment(.center)

                Spacer()
            }
            .padding(24)
        }
    }
}
