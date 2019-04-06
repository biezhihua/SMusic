package com.bzh.splayer.lib.common

import android.content.Context
import java.io.BufferedReader
import java.io.IOException
import java.io.InputStreamReader

object RawResourceReader {
    fun readTextFileFromRawResource(
        context: Context,
        resourceId: Int
    ): String? {
        val inputStream = context.resources.openRawResource(
            resourceId
        )
        val inputStreamReader = InputStreamReader(
            inputStream
        )
        val bufferedReader = BufferedReader(
            inputStreamReader
        )

        val body = StringBuilder()

        try {
            bufferedReader.forEachLine {
                body.append(it)
                body.append('\n')
            }
        } catch (e: IOException) {
            return null
        }

        return body.toString()
    }
}
