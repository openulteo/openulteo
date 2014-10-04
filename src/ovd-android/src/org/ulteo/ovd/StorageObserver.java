/*
 * Copyright (C) 2012-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2012-2013
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

package org.ulteo.ovd;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;
import android.os.Environment;
import android.util.Log;

public class StorageObserver {
	
	public static class StorageEntry {
		public String name;
		public String path;
		
		public StorageEntry(String aname, String apath) {
			name = aname;
			path = apath;
		}
		
		public StorageEntry(String apath) {
			name = (new File(apath)).getName();
			path = apath;
		}
	}

	public static ArrayList<StorageEntry> getStorageDirectories() {
		ArrayList<StorageEntry> list = new ArrayList<StorageEntry>();
		BufferedReader bufReader = null;
		String sdcard = Environment.getExternalStorageDirectory().getPath();
		try {
			bufReader = new BufferedReader(new FileReader("/proc/mounts"));
			if (Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState())) {
				list.add(new StorageEntry(Config.SDCARD, sdcard));
			}
			String line;
			while ((line = bufReader.readLine()) != null) {
				String[] parts = line.split(" +");
				if (parts.length < 3) {
					continue;
				}
				String device = parts[0];
				String mountPoint = parts[1];
				String fsType = parts[2];
				if (!fsType.equals("vfat") && !fsType.equals("tntfs")) {
					continue;
				}
				if (mountPoint.startsWith("/mnt/secure") || mountPoint.startsWith("/mnt/asec")
						|| mountPoint.startsWith("/mnt/obb") || device.startsWith("/dev/mapper")) {
					continue;
				}
				if (mountPoint.equals(sdcard)) {
					continue;
				}
				if (!device.contains("/dev/block/vold"))
					continue;
				list.add(new StorageEntry(mountPoint));
			}
			bufReader.close();
		} catch (Exception e) {
			Log.e(Config.TAG, "getStorageDirectories " + e.getMessage(), e);
		}
		return list;
	}
}
