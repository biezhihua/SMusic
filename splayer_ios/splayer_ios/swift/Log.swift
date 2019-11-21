import Foundation

public class Log {

    public static func d(_ tag: String, _ message: String) {
        NSLog("%@ %@", tag, message)
    }
}
