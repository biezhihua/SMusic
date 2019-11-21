import SwiftUI
import splayer_ios

struct MainView: View {

    private let TAG: String = "[MP][UI][MainView]"

    var body: some View {
        contentView()
    }

    private func contentView() -> some View {
        var mediaPlayer: MediaPlayer?
        return VStack(alignment: HorizontalAlignment.leading) {
            HStack {
                Button("Create") {
                    Log.d(self.TAG, "Create")
                    mediaPlayer = MediaPlayer()
                }.buttonStyle(MyButtonStyle())

                Button("Destroy") {
                    Log.d(self.TAG, "Destroy")
                    mediaPlayer?.release()
                    mediaPlayer = nil
                }.buttonStyle(MyButtonStyle())

                Button("Start") {
                    Log.d(self.TAG, "Start")
                    mediaPlayer?.start()
                }.buttonStyle(MyButtonStyle())

                Button("Stop") {
                    Log.d(self.TAG, "Stop")
                    mediaPlayer?.stop()
                }.buttonStyle(MyButtonStyle())

                Button("Play") {
                    Log.d(self.TAG, "Play")
                    mediaPlayer?.play()
                }.buttonStyle(MyButtonStyle())

                Button("Pause") {
                    Log.d(self.TAG, "Pause")
                    mediaPlayer?.pause()
                }.buttonStyle(MyButtonStyle())

            }

            Spacer()
        }.padding()
    }
}

struct MyButtonStyle: ButtonStyle {

    func makeBody(configuration: Self.Configuration) -> some View {
        configuration.label
                .padding(EdgeInsets(top: 3, leading: 6, bottom: 3, trailing: 6))
                .foregroundColor(.white)
                .background(configuration.isPressed ? Color.red : Color.blue)
                .cornerRadius(8.0)
    }

}


struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        MainView()
    }
}
