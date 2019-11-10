import Foundation


class Log {

    public static func d(_ tag: String, _ message: String) {
        NSLog("%@ %@", tag, message)
    }
}
