package com.example.zjf.androidopenslaudio;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

	String BASE_PATH = Environment.getExternalStorageDirectory().getPath();
	private EditText etUrl;
	private Button btnPlay;
	static {
		System.loadLibrary("native-lib");
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		etUrl = (EditText) findViewById(R.id.etUrl);
		btnPlay = (Button) findViewById(R.id.btnPlay);
	}

	public native void palyPcm(String url);

	public void play(View view) {
		String path = BASE_PATH + "/" + etUrl.getText().toString();
		Log.d("ZJF",path);
		palyPcm(path);
	}
}
