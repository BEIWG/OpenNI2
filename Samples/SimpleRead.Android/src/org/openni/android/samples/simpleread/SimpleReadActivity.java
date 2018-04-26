/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
package org.openni.android.samples.simpleread;

import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import org.openni.Device;
import org.openni.DeviceInfo;
import org.openni.OpenNI;
import org.openni.SensorType;
import org.openni.VideoFrameRef;
import org.openni.VideoStream;
import org.openni.android.OpenNIHelper;

public class SimpleReadActivity 
		extends Activity 
		implements OpenNIHelper.DeviceOpenListener {
	
	private static final String TAG = "SimplerRead";
	private OpenNIHelper mOpenNIHelper;
	private boolean mDeviceOpenPending = false;
	private Thread mMainLoopThread;
	private boolean mShouldRun = true;
	private Device mDevice;
	private VideoStream mStream_depth;
	private VideoStream mStream_rgb;
	private TextView mStatusLine;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		mOpenNIHelper = new OpenNIHelper(this);
		OpenNI.setLogAndroidOutput(true);
		OpenNI.setLogMinSeverity(0);
		OpenNI.initialize();
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_simple_read);
		mStatusLine = (TextView) findViewById(R.id.status_line);
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		
		mOpenNIHelper.shutdown();
		OpenNI.shutdown();
	}

	@Override
	protected void onResume() {
		Log.d(TAG, "onResume");

		super.onResume();

		// onResume() is called after the USB permission dialog is closed, in which case, we don't want
		// to request device permissions again
		if (mDeviceOpenPending) {
			return;
		}

		// Request opening the first OpenNI-compliant device found
		String uri;
		
		List<DeviceInfo> devices = OpenNI.enumerateDevices();
		if (devices.isEmpty()) {
			showAlertAndExit("No OpenNI-compliant device found.");
			return;
		}
		
		uri = devices.get(0).getUri();
		
		mDeviceOpenPending = true;
		mOpenNIHelper.requestDeviceOpen(uri, this);
	}

	private void showAlertAndExit(String message) {
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setMessage(message);
		builder.setNeutralButton("OK", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				finish();
			}
		});
		builder.show();
	}

	@Override
	public void onDeviceOpened(Device aDevice) {
		Log.d(TAG, "Permission granted for device " + aDevice.getDeviceInfo().getUri());
		
		mDeviceOpenPending = false;

		try {
			mDevice = aDevice;
			mStream_depth = VideoStream.create(mDevice, SensorType.DEPTH);
			mStream_depth.start();
			
			mStream_rgb = VideoStream.create(mDevice, SensorType.COLOR);
			mStream_rgb.start();
		} catch (RuntimeException e) {
			showAlertAndExit("Failed to open stream: " + e.getMessage());
			return;
		}
		
		mShouldRun = true;
		mMainLoopThread = new Thread() {
			@Override
			public void run() {
				while (mShouldRun) {
					VideoFrameRef frame_depth = null;
					VideoFrameRef frame_rgb = null;
					
					try {
						frame_depth = mStream_depth.readFrame();
						frame_rgb = mStream_rgb.readFrame();
						// get the middle pixel
						int index = frame_depth.getVideoMode().getResolutionY() / 2 * frame_depth.getVideoMode().getResolutionX() + frame_depth.getVideoMode().getResolutionX() / 2;
						short depthValue = frame_depth.getData().getShort(index * 2);
						
						      index = frame_rgb.getVideoMode().getResolutionY() / 2 * frame_rgb.getVideoMode().getResolutionX() + frame_rgb.getVideoMode().getResolutionX() / 2;
						 int rgbValue = frame_rgb.getData().getShort(index * 2);
						updateLabel(String.format("Depth->Frame: %,d \n Timestamp: %.3f seconds \n depth value: %,d mm\n" + "--------------\n" +
								"RGB->Frame: %,d \n Timestamp: %.3f seconds \n rgb value: %,d", frame_depth.getFrameIndex(), frame_depth.getTimestamp() / 1e3, depthValue, 
								frame_rgb.getFrameIndex(), frame_rgb.getTimestamp() / 1e3, rgbValue&0xff));
					} catch (Exception e) {
						Log.e(TAG, "Failed reading frame_depth: " + e);
					}
				}
			};
		};
		
		mMainLoopThread.setName("SimpleRead MainLoop Thread");
		mMainLoopThread.start();
	}
	
	private void stop() {
		mShouldRun = false;
		
		while (mMainLoopThread != null) {
			try {
				mMainLoopThread.join();
				mMainLoopThread = null;
				break;
			} catch (InterruptedException e) {
			}
		}

		if (mStream_depth != null) {
			mStream_depth.stop();
		}

		if (mStream_rgb != null) {
			mStream_rgb.stop();
		}
		
		mStatusLine.setText(R.string.waiting_for_frames);
	}
	
	private void updateLabel(final String message) {
		runOnUiThread(new Runnable() {
			public void run() {
				mStatusLine.setText(message);								
			}
		});
	}

	@Override
	public void onDeviceOpenFailed(String uri) {
		Log.e(TAG, "Failed to open device " + uri);
		mDeviceOpenPending = false;
		showAlertAndExit("Failed to open device");
	}

	@Override
	protected void onPause() {
		Log.d(TAG, "onPause");

		super.onPause();
		
		// onPause() is called just before the USB permission dialog is opened, in which case, we don't
		// want to shutdown OpenNI
		if (mDeviceOpenPending)
			return;

		stop();
		
		if (mStream_depth != null) {
			mStream_depth.destroy();
			mStream_depth = null;
		}

		if (mStream_rgb != null) {
			mStream_rgb.destroy();
			mStream_rgb = null;
		}
		
		if (mDevice != null) {
			mDevice.close();
			mDevice = null;
		}
	}
}
