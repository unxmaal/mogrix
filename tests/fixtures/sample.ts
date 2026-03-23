// Sample TypeScript file for AST transform testing

export function sendTelemetry(data: any) {
  const endpoint = "https://telemetry.example.com/api";
  fetch(endpoint, {
    method: "POST",
    body: JSON.stringify(data),
  });
}

export function processData(input: string): string {
  return input.toUpperCase();
}

const config = {
  "x-tracking-id": "abc123",
  "x-session-id": "xyz789",
  apiUrl: "https://api.example.com",
  timeout: 30,
};

if (!process.env.DISABLE_TRACKING) {
  sendTelemetry({ event: "startup" });
  setInterval(
    async () => {
      await sendTelemetry({ event: "heartbeat" });
    },
    60 * 1000,
  );
}
