package org.ppsspp.ppsspp;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Build;
import android.os.PowerManager;
import android.provider.Settings;
import android.util.Log;

public class PowerSaveModeReceiver extends BroadcastReceiver {
	private static final String TAG = PowerSaveModeReceiver.class.getSimpleName();
	private static boolean isBatteryLow = false;
	private final ContentObserver settingsObserver;
	private Object thermalStatusListener = null;

	@Override
	public void onReceive(final Context context, final Intent intent) {
		final String action = intent.getAction();
		if (Intent.ACTION_BATTERY_LOW.equals(action)) {
			isBatteryLow = true;
		} else if (Intent.ACTION_BATTERY_OKAY.equals(action)) {
			isBatteryLow = false;
		} else if (PowerManager.ACTION_POWER_SAVE_MODE_CHANGED.equals(action)) {
			// sendPowerSaving()
		}

		sendPowerSaving(context);
	}

	public PowerSaveModeReceiver(final Activity activity) {
		IntentFilter filter = new IntentFilter();
		filter.addAction(Intent.ACTION_BATTERY_LOW);
		filter.addAction(Intent.ACTION_BATTERY_OKAY);
		filter.addAction(PowerManager.ACTION_POWER_SAVE_MODE_CHANGED);
		activity.registerReceiver(this, filter);

		settingsObserver = new ContentObserver(null) {
			@Override
			public void onChange(boolean selfChange, Uri uri) {
				super.onChange(selfChange, uri);

				String key = uri.getPath();
				if (key == null) {
					return;
				}
				key = key.substring(key.lastIndexOf("/") + 1);
				if (key.equals("user_powersaver_enable") || key.equals("psm_switch") || key.equals("powersaving_switch")) {
					sendPowerSaving(activity);
				}
			}
		};
		activity.getContentResolver().registerContentObserver(Settings.System.CONTENT_URI, true, settingsObserver);
		sendPowerSaving(activity);
		registerThermalListener(activity);
	}

	private void registerThermalListener(final Context context) {
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
			return;
		}
		try {
			final PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
			if (pm == null) {
				return;
			}
			sendThermalStatus(pm.getCurrentThermalStatus());
			PowerManager.OnThermalStatusChangedListener listener = status -> sendThermalStatus(status);
			pm.addThermalStatusListener(listener);
			thermalStatusListener = listener;
		} catch (Exception e) {
			Log.w(TAG, "Failed to register thermal listener: " + e);
		}
	}

	private void sendThermalStatus(int status) {
		if (!PpssppActivity.libraryLoaded) {
			return;
		}
		try {
			NativeApp.sendMessageFromJava("device_thermalStatus", Integer.toString(status));
		} catch (Exception e) {
			Log.w(TAG, "Failed sending thermal status: " + e);
		}
	}

	public void destroy(final Context context) {
		context.unregisterReceiver(this);
		// The observer holds on to the activity, and the system holds on to a registered observer.
		context.getContentResolver().unregisterContentObserver(settingsObserver);
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q && thermalStatusListener != null) {
			try {
				final PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
				if (pm != null) {
					pm.removeThermalStatusListener((PowerManager.OnThermalStatusChangedListener) thermalStatusListener);
				}
			} catch (Exception e) {
				Log.w(TAG, "Failed to remove thermal listener: " + e);
			}
			thermalStatusListener = null;
		}
	}

	private static boolean getNativePowerSaving(final Context context) {
		final PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
		return pm.isPowerSaveMode();
	}

	protected void sendPowerSaving(final Context context) {
		boolean isPowerSaving = getNativePowerSaving(context);

		if (!PpssppActivity.libraryLoaded) {
			Log.e(TAG, "Cannot send power saving: Library not loaded");
			return;
		}

		try {
			if (isBatteryLow || isPowerSaving) {
				NativeApp.sendMessageFromJava("core_powerSaving", "true");
			} else {
				NativeApp.sendMessageFromJava("core_powerSaving", "false");
			}
		} catch (Exception e) {
			Log.e(TAG, "Exception in sendPowerSaving: " + e);
		}
	}
}
