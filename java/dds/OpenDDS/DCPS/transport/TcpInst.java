/*
 * $Id$
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

package OpenDDS.DCPS.transport;

public class TcpInst extends TransportInst {

    TcpInst(int id) {
        super(id);
    }

    native void saveSpecificConfig(long cfg);
    native void loadSpecificConfig(long cfg);

    public String getType() { return "Tcp"; }

    private String localAddress;
    public String getLocalAddress() { return localAddress; }
    public void setLocalAddress(String la) { localAddress = la; }

    private boolean enableNagleAlgorithm;
    public boolean isEnableNagleAlgorithm() { return enableNagleAlgorithm; }
    public void setEnableNagleAlgorithm(boolean ena) {
        enableNagleAlgorithm = ena;
    }

    private int connRetryInitialDelay;
    public int getConnRetryInitialDelay() { return connRetryInitialDelay; }
    public void setConnRetryInitialDelay(int crid) {
        connRetryInitialDelay = crid;
    }

    private double connRetryBackoffMultiplier;
    public double getConnRetryBackoffMultiplier() {
        return connRetryBackoffMultiplier;
    }
    public void setConnRetryBackoffMultiplier(double crbm) {
        connRetryBackoffMultiplier = crbm;
    }

    private int connRetryAttempts;
    public int getConnRetryAttempts() { return connRetryAttempts; }
    public void setConnRetryAttempts(int cra) { connRetryAttempts = cra; }

    private int maxOutputPausePeriod;
    public int getMaxOutputPausePeriod() { return maxOutputPausePeriod; }
    public void setMaxOutputPausePeriod(int mopp) {
        maxOutputPausePeriod = mopp;
    }

    private int passiveReconnectDuration;
    public int getPassiveReconnectDuration() {
        return passiveReconnectDuration;
    }
    public void setPassiveReconnectDuration(int prd) {
        passiveReconnectDuration = prd;
    }

    private int passiveConnectDuration;
    public int getPassiveConnectDuration() { return passiveConnectDuration; }
    public void setPassiveConnectDuration(int pcd) {
        passiveConnectDuration = pcd;
    }
}